#include "NetTPSCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Components/SceneComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "NetPlayerAnimInstance.h"
#include "MainUI.h"
#include "Components/WidgetComponent.h"
#include "HealthBar.h"
#include "GameFramework/PlayerController.h"
#include "NetTPS.h"
#include "Net/UnrealNetwork.h"
#include "Components/HorizontalBox.h"
#include "NetPlayerController.h"

DEFINE_LOG_CATEGORY(LogTemplateCharacter);

ANetTPSCharacter::ANetTPSCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false; 
	GetCharacterMovement()->bUseControllerDesiredRotation = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); 

	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 150.f;
	CameraBoom->SetRelativeLocation(FVector(0.0f, 40.0f, 60.0f));
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); 
	FollowCamera->bUsePawnControlRotation = false; 


	GunComp=CreateDefaultSubobject<USceneComponent>(TEXT("GunComp"));
	GunComp->SetupAttachment(GetMesh(), TEXT("GunPosition"));
	GunComp->SetRelativeLocation(FVector(3.f,-7.f, -1.f)); //	(X = 3.964429, Y = -7.864156, Z = -1.275523)
	GunComp->SetRelativeRotation(FRotator(22.f, 79.f, -15.f)); // (Pitch=22.334616,Yaw=79.078269,Roll=-15.903337)

	// HealthBar Component
	HpUIComp = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBar"));
	HpUIComp->SetupAttachment(GetMesh());


}

void ANetTPSCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// locallyconrolled가 있고 
	if (IsLocallyControlled() && !HasAuthority()) {
		InitUIWidget();
	}

	//  총 검색 
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors); // 월드에 있는 모든 액터들이 리스트에 쭉 들어온다
	for (auto TempPistol : AllActors) {
		if (TempPistol->GetActorNameOrLabel().Contains("BP_Pistol")) {
			PistolActors.Add(TempPistol);
		}
	}

}

void ANetTPSCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ANetTPSCharacter::TakePistol(const FInputActionValue& Value)
{
	// 총을 소유하지 않다면 일정 범위 안에 있는 총을 잡는다
	// 필요 속성 : 총을 소유하고 있는지, 소유 중인 총, 총을 잡을 수 있는 범위
	// 1. 총을 잡고 있지 않다면
	if (bHasPistol) {
		return;
	}
	// 클라에서 서버로 요청 (여기는 클라)
	ServerRPC_TakePistol();
}

void ANetTPSCharacter::AttachPistol(AActor* pistolActor)
{
	auto meshComp = pistolActor->GetComponentByClass<UStaticMeshComponent>();
	meshComp->SetSimulatePhysics(false);
	meshComp->AttachToComponent(GunComp, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
	
	if (IsLocallyControlled() && MainUI) {
		MainUI->ShowCrosshair(true); 
	}
}

void ANetTPSCharacter::ReleasePistol(const FInputActionValue& Value)
{
	// 총을 잡지 않거나 재장전 중이라면 처리하지 않는다
	if (!bHasPistol || bIsReloading || !IsLocallyControlled())return;

	ServerRPC_ReleasePistol();
}

void ANetTPSCharacter::DetachPistol(AActor* pistolActor)
{
	auto meshComp = pistolActor->GetComponentByClass<UStaticMeshComponent>();
	meshComp->SetSimulatePhysics(true);
	meshComp->DetachFromComponent(FDetachmentTransformRules::KeepRelativeTransform);
	
	if (IsLocallyControlled() && MainUI) {
		MainUI->ShowCrosshair(false);
	}
}

void ANetTPSCharacter::Fire(const FInputActionValue& Value)
{
	// 총을 들고 있지 않거나 재장전 중이거나 총알이 없을 경우, 처리하지 않는다
	if (!bHasPistol || bIsReloading || BulletCount <= 0) return;

	ServerRPC_Fire();
}

void ANetTPSCharacter::ReloadPistol(const FInputActionValue& Value)
{
	// 총 소지중이 아니라면 아무 처리하지 않는다
	if (!bHasPistol || bIsReloading) return;
	
	// 재장전 애니메이션 재생
	auto anim = Cast<UNetPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	anim->PlayReloadAnimation();
	bIsReloading = true;
}

void ANetTPSCharacter::InitAmmonUI()
{
	ServerRPC_Reload();
}

void ANetTPSCharacter::OnRep_HP()
{
	// 사망처리
	if (HP <= 0) {
		bIsDead = true;
		ReleasePistol(FInputActionValue());
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 땅속으로 죽어버리기
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCharacterMovement()->DisableMovement();
	}

	// UI에 할당할 퍼센트 계산
	float perscent = hp / MaxHP;

	if (MainUI) {
		MainUI->HP = perscent;
		// 피격효과 처리
		MainUI->PlayDamageAnimation();

		if (DamageCameraShake) {
			auto pc = Cast<APlayerController>(Controller);
			pc->ClientStartCameraShake(DamageCameraShake);
		}
	}
	else {
		auto HpUI = Cast<UHealthBar>(HpUIComp->GetWidget());
		HpUI->HP = perscent;
	}
}

float ANetTPSCharacter::GetHP()
{
	return hp;
}

void ANetTPSCharacter::SetHP(float value)
{
	hp = value;
	OnRep_HP();
}

void ANetTPSCharacter::DamageProcess()
{
	// 체력을 감소시킨다
	HP--;
}

void ANetTPSCharacter::DieProcess()
{
	auto pc = Cast<APlayerController>(Controller);
	pc->SetShowMouseCursor(true);
	GetFollowCamera()->PostProcessSettings.ColorSaturation = FVector4(0.f, 0.f, 0.f, 1.f);
	
	// Die UI 표시
	if (MainUI) {
		MainUI->GameOverUI->SetVisibility(ESlateVisibility::Visible);
	}
}

void ANetTPSCharacter::PossessedBy(AController* NewController)
{
	PRINTLOG(TEXT("Begin"));
	Super::PossessedBy(NewController);

	if (IsLocallyControlled()) {
		InitUIWidget();
	}

	PRINTLOG(TEXT("End"));
}

void ANetTPSCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// HPBar 빌보드
	if (HpUIComp && HpUIComp->GetVisibleFlag()) {
		FVector CamLoc = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraLocation(); // 자기자신
		FVector Direction = CamLoc - HpUIComp->GetComponentLocation();
		Direction.Z = 0.f;

		HpUIComp->SetWorldRotation(Direction.GetSafeNormal().ToOrientationRotator());
	}

	PrintNetLog();
}

void ANetTPSCharacter::PrintNetLog()
{
	const FString conStr = GetNetConnection() != nullptr ? TEXT("Valid Connection") : TEXT("Invalid Connection");
	const FString ownerName = GetOwner() != nullptr ? GetOwner()->GetName() : TEXT("No Owner");
	const FString logStr = FString::Printf(TEXT("Connection : %s\nOwner Name : %s\nLocal Role : %s\nRemote Role : %s"), *conStr, *ownerName, *LOCAL_ROLE, *REMOTE_ROLE);
	DrawDebugString(GetWorld(), GetActorLocation() + FVector::UpVector * 100.f, logStr, nullptr, FColor::White, 0, true, 1);

	// 권한(Authority)
	// ROLE_Authority			 :		모든 권한을 다 가지고 있다 (로직 실행 가능)
	// HasAuthority()			 :		서버인지 클라인지
					
	// ROLE_AutonomouseProxy	 :		제어(Input)만 가능
	// IsLocallyControlled()     :      PlayerController가 Possess 중인지
	
	// ROLE_SimulatedProxy		 :		시뮬레이션만 가능
}

void ANetTPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ANetTPSCharacter::Move);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ANetTPSCharacter::Look);

		EnhancedInputComponent->BindAction(TakePistolAction, ETriggerEvent::Started, this, &ThisClass::TakePistol);

		EnhancedInputComponent->BindAction(ReleaseAction, ETriggerEvent::Started, this, &ThisClass::ReleasePistol);

		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &ThisClass::Fire);

		EnhancedInputComponent->BindAction(ReloadAction, ETriggerEvent::Started, this, &ThisClass::ReloadPistol);
	}
	else
	{
		UE_LOG(LogTemplateCharacter, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void ANetTPSCharacter::ServerRPC_TakePistol_Implementation()
{
	// 클라에서 요청받은 총잡기를 처리한다 (여기는 서버)
	// 2. 월드에 있는 총을 모두 찾는다
	for (auto pistolActor : PistolActors) {
		// 3. 총의 주인이 있다면 그 총은 검사하지 않는다
		if (pistolActor->GetOwner() != nullptr) {
			continue;
		}
		// 4. 총과의 거리를 구한다
		float Distance = FVector::Dist(GetActorLocation(), pistolActor->GetActorLocation());
		// 5. 총이 범위 안에 있다면 
		if (Distance > DistanceToGun) {
			continue;
		}

		// 6. 소유중인 총으로 등록
		OwnedPistol = pistolActor;

		// 7. 총의 소유자를 자신으로 등록
		OwnedPistol->SetOwner(this);

		// 8. 총 소유상태를 변경
		bHasPistol = true;

		// 총을 잡으라고 클라에 요청 (여기는 서버)
		MulticastRPC_TakePistol(pistolActor);
		break; // 총 꺼냈으니까 돌아야할 필요X
	}
}

void ANetTPSCharacter::MulticastRPC_TakePistol_Implementation(AActor* pistolActor)
{
	// 서버에서 인자로 넘어온 총 액터를 붙이자 (여기는 클라)
	// 9. 총 붙이기
	AttachPistol(pistolActor);
}

void ANetTPSCharacter::ServerRPC_ReleasePistol_Implementation()
{
	// 총 소유시
	if (OwnedPistol) {
		MulticastRPC_ReleasePistol(OwnedPistol);

		// 미소유로 설정
		bHasPistol = false;
		OwnedPistol->SetOwner(nullptr);
		OwnedPistol = nullptr;
	}
}

void ANetTPSCharacter::MulticastRPC_ReleasePistol_Implementation(AActor* pistolActor)
{
	// 총 분리
	DetachPistol(pistolActor);
}

void ANetTPSCharacter::ServerRPC_Fire_Implementation()
{
	// 총알 제거
	BulletCount--;

	// 총쏘기
	FHitResult HitInfo;
	FVector StartPos = FollowCamera->GetComponentLocation();
	FVector EndPos = StartPos + FollowCamera->GetForwardVector() * 10000.f;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitInfo, StartPos, EndPos, ECollisionChannel::ECC_Visibility, Params);
	if (bHit) {
		// 맞은 대상이 상대방일 경우 데미지 처리
		auto otherPlayer = Cast<ANetTPSCharacter>(HitInfo.GetActor());
		if (otherPlayer) {
			otherPlayer->DamageProcess();
		}
	}

	MulticastRPC_Fire(bHit, HitInfo);
}

void ANetTPSCharacter::MulticastRPC_Fire_Implementation(bool bHit, const FHitResult& HitInfo)
{
	if (bHit) {
		// 맞은 부위에 particle 표시
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), GunEffect, HitInfo.Location, FRotator(), true);
	}

	if (MainUI) {
		MainUI->PopBullet(BulletCount);
	}

	// 총쏘기 애니메이션 진행
	auto anim = Cast<UNetPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	anim->PlayFireAnimation();
}

void ANetTPSCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ANetTPSCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ANetTPSCharacter::InitUIWidget()
{
	PRINTLOG(TEXT("[%s] Begin"), Controller ? TEXT("PLAYER") : TEXT("Not Player"));
	
	// 플레이어가 제어중이 아니라면 처리하지 않는다 = playercontroller가 없다면 처리하지 X
	auto pc = Cast<ANetPlayerController>(Controller);
	if (pc == nullptr)return;

	if (pc->MainUIWidget) {
		if (pc->MainUI == nullptr) {
			pc->MainUI = Cast<UMainUI>(CreateWidget(GetWorld(), pc->MainUIWidget));
		}
		MainUI = pc->MainUI;
		MainUI->AddToViewport();
		MainUI->ShowCrosshair(false);

		hp = MaxHP;
		MainUI->HP = 1.f;

		// 총알 모두 제거
		MainUI->RemoveAllAmmo();
		BulletCount = MaxBulletCount;

		// MaxBulleCount만큼 총알추가
		for (int i = 0;i < MaxBulletCount;++i) {
			MainUI->AddBullet();
		}

		// MainUI가 있기 때문에 해당 컴포넌트는 비활성화
		if (HpUIComp) {
			HpUIComp->SetVisibility(false);
		}
	}
}

void ANetTPSCharacter::ServerRPC_Reload_Implementation()
{
	// 총알 개수를 초기화
	BulletCount = MaxBulletCount;
	ClientRPC_Reload();
}

void ANetTPSCharacter::ClientRPC_Reload_Implementation()
{
	if (MainUI) {
		// 총알 UI 제거
		MainUI->RemoveAllAmmo();
		// 총알 UI 다시 세팅
		for (int i = 0;i < MaxBulletCount;++i) {
			MainUI->AddBullet();
		}
	}
	// 재장전 완료상태로 처리
	bIsReloading = false;
}

void ANetTPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetTPSCharacter, bHasPistol);
	DOREPLIFETIME(ANetTPSCharacter, BulletCount);
	DOREPLIFETIME(ANetTPSCharacter, hp);
}