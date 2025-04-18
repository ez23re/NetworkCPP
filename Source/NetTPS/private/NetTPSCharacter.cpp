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
	
	// locallyconrolled�� �ְ� 
	if (IsLocallyControlled() && !HasAuthority()) {
		InitUIWidget();
	}

	//  �� �˻� 
	TArray<AActor*> AllActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AActor::StaticClass(), AllActors); // ���忡 �ִ� ��� ���͵��� ����Ʈ�� �� ���´�
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
	// ���� �������� �ʴٸ� ���� ���� �ȿ� �ִ� ���� ��´�
	// �ʿ� �Ӽ� : ���� �����ϰ� �ִ���, ���� ���� ��, ���� ���� �� �ִ� ����
	// 1. ���� ��� ���� �ʴٸ�
	if (bHasPistol) {
		return;
	}
	// Ŭ�󿡼� ������ ��û (����� Ŭ��)
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
	// ���� ���� �ʰų� ������ ���̶�� ó������ �ʴ´�
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
	// ���� ��� ���� �ʰų� ������ ���̰ų� �Ѿ��� ���� ���, ó������ �ʴ´�
	if (!bHasPistol || bIsReloading || BulletCount <= 0) return;

	ServerRPC_Fire();
}

void ANetTPSCharacter::ReloadPistol(const FInputActionValue& Value)
{
	// �� �������� �ƴ϶�� �ƹ� ó������ �ʴ´�
	if (!bHasPistol || bIsReloading) return;
	
	// ������ �ִϸ��̼� ���
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
	// ���ó��
	if (HP <= 0) {
		bIsDead = true;
		ReleasePistol(FInputActionValue());
		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision); // �������� �׾������
		GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCharacterMovement()->DisableMovement();
	}

	// UI�� �Ҵ��� �ۼ�Ʈ ���
	float perscent = hp / MaxHP;

	if (MainUI) {
		MainUI->HP = perscent;
		// �ǰ�ȿ�� ó��
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
	// ü���� ���ҽ�Ų��
	HP--;
}

void ANetTPSCharacter::DieProcess()
{
	auto pc = Cast<APlayerController>(Controller);
	pc->SetShowMouseCursor(true);
	GetFollowCamera()->PostProcessSettings.ColorSaturation = FVector4(0.f, 0.f, 0.f, 1.f);
	
	// Die UI ǥ��
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

	// HPBar ������
	if (HpUIComp && HpUIComp->GetVisibleFlag()) {
		FVector CamLoc = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0)->GetCameraLocation(); // �ڱ��ڽ�
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

	// ����(Authority)
	// ROLE_Authority			 :		��� ������ �� ������ �ִ� (���� ���� ����)
	// HasAuthority()			 :		�������� Ŭ������
					
	// ROLE_AutonomouseProxy	 :		����(Input)�� ����
	// IsLocallyControlled()     :      PlayerController�� Possess ������
	
	// ROLE_SimulatedProxy		 :		�ùķ��̼Ǹ� ����
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
	// Ŭ�󿡼� ��û���� ����⸦ ó���Ѵ� (����� ����)
	// 2. ���忡 �ִ� ���� ��� ã�´�
	for (auto pistolActor : PistolActors) {
		// 3. ���� ������ �ִٸ� �� ���� �˻����� �ʴ´�
		if (pistolActor->GetOwner() != nullptr) {
			continue;
		}
		// 4. �Ѱ��� �Ÿ��� ���Ѵ�
		float Distance = FVector::Dist(GetActorLocation(), pistolActor->GetActorLocation());
		// 5. ���� ���� �ȿ� �ִٸ� 
		if (Distance > DistanceToGun) {
			continue;
		}

		// 6. �������� ������ ���
		OwnedPistol = pistolActor;

		// 7. ���� �����ڸ� �ڽ����� ���
		OwnedPistol->SetOwner(this);

		// 8. �� �������¸� ����
		bHasPistol = true;

		// ���� ������� Ŭ�� ��û (����� ����)
		MulticastRPC_TakePistol(pistolActor);
		break; // �� �������ϱ� ���ƾ��� �ʿ�X
	}
}

void ANetTPSCharacter::MulticastRPC_TakePistol_Implementation(AActor* pistolActor)
{
	// �������� ���ڷ� �Ѿ�� �� ���͸� ������ (����� Ŭ��)
	// 9. �� ���̱�
	AttachPistol(pistolActor);
}

void ANetTPSCharacter::ServerRPC_ReleasePistol_Implementation()
{
	// �� ������
	if (OwnedPistol) {
		MulticastRPC_ReleasePistol(OwnedPistol);

		// �̼����� ����
		bHasPistol = false;
		OwnedPistol->SetOwner(nullptr);
		OwnedPistol = nullptr;
	}
}

void ANetTPSCharacter::MulticastRPC_ReleasePistol_Implementation(AActor* pistolActor)
{
	// �� �и�
	DetachPistol(pistolActor);
}

void ANetTPSCharacter::ServerRPC_Fire_Implementation()
{
	// �Ѿ� ����
	BulletCount--;

	// �ѽ��
	FHitResult HitInfo;
	FVector StartPos = FollowCamera->GetComponentLocation();
	FVector EndPos = StartPos + FollowCamera->GetForwardVector() * 10000.f;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitInfo, StartPos, EndPos, ECollisionChannel::ECC_Visibility, Params);
	if (bHit) {
		// ���� ����� ������ ��� ������ ó��
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
		// ���� ������ particle ǥ��
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), GunEffect, HitInfo.Location, FRotator(), true);
	}

	if (MainUI) {
		MainUI->PopBullet(BulletCount);
	}

	// �ѽ�� �ִϸ��̼� ����
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
	
	// �÷��̾ �������� �ƴ϶�� ó������ �ʴ´� = playercontroller�� ���ٸ� ó������ X
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

		// �Ѿ� ��� ����
		MainUI->RemoveAllAmmo();
		BulletCount = MaxBulletCount;

		// MaxBulleCount��ŭ �Ѿ��߰�
		for (int i = 0;i < MaxBulletCount;++i) {
			MainUI->AddBullet();
		}

		// MainUI�� �ֱ� ������ �ش� ������Ʈ�� ��Ȱ��ȭ
		if (HpUIComp) {
			HpUIComp->SetVisibility(false);
		}
	}
}

void ANetTPSCharacter::ServerRPC_Reload_Implementation()
{
	// �Ѿ� ������ �ʱ�ȭ
	BulletCount = MaxBulletCount;
	ClientRPC_Reload();
}

void ANetTPSCharacter::ClientRPC_Reload_Implementation()
{
	if (MainUI) {
		// �Ѿ� UI ����
		MainUI->RemoveAllAmmo();
		// �Ѿ� UI �ٽ� ����
		for (int i = 0;i < MaxBulletCount;++i) {
			MainUI->AddBullet();
		}
	}
	// ������ �Ϸ���·� ó��
	bIsReloading = false;
}

void ANetTPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetTPSCharacter, bHasPistol);
	DOREPLIFETIME(ANetTPSCharacter, BulletCount);
	DOREPLIFETIME(ANetTPSCharacter, hp);
}