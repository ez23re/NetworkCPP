#include "NetActor.h"
#include "Components/StaticMeshComponent.h"
#include "NetTPS.h"
#include "NetTPSCharacter.h"
#include "EngineUtils.h"
#include "Net/UnrealNetwork.h"
#include "UObject/CoreNet.h"
#include "Engine/TimerHandle.h"
#include "TimerManager.h"
#include "Math/Color.h"

ANetActor::ANetActor()
{
	PrimaryActorTick.bCanEverTick = true;
	
	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComp"));
	SetRootComponent(MeshComp);
	MeshComp->SetRelativeScale3D(FVector(0.5f));

	bReplicates = true;

	// �뿪�� ����
	NetUpdateFrequency = 1.f;

}

void ANetActor::BeginPlay()
{
	Super::BeginPlay();
	
	Mat = MeshComp->CreateDynamicMaterialInstance(0);

	if (HasAuthority()) {
		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(Handle,
			FTimerDelegate::CreateLambda([&]
				{
					MatColor = FLinearColor(FMath::RandRange(0.f,0.3f),
											FMath::RandRange(0.f, 0.3f),
											FMath::RandRange(0.f, 0.3f),1.f);
					OnRep_ChangeMatColor();
				}
			), 1, true);
		}

}

void ANetActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FindOwner();
	PrintNetLog();

	// ������ ���
	if (HasAuthority()) {
		// ���� ����
		AddActorLocalRotation(FRotator(0.f, 50.f * DeltaTime, 0.f));
		RotYaw = GetActorRotation().Yaw;
	}
	else {
		// Ŭ���̾�Ʈ ��ü ����
		// ����ð� ����
		currentTime += DeltaTime;
		// 0���� �������� �ʵ��� lastTime �� üũ
		if (lastTime < KINDA_SMALL_NUMBER) {
			return;
		}

		// ���� ����ð��� ���� ����ð��� �������
		float lerpRatio = currentTime / lastTime;
		// ���� ����ð���ŭ ȸ���� ������ ���ο� ȸ���� ���
		float newYaw = RotYaw + 50 * lastTime;
		// �����Ǵ� ������ ����� �ð���ŭ ����ó��
		float lerpYaw = FMath::Lerp(RotYaw, newYaw, lerpRatio);
		// ���� ����
		FRotator CurRot = GetActorRotation();
		CurRot.Yaw = lerpYaw;
		SetActorRotation(CurRot);
	}

	//else {
	//	// Ŭ���̾�Ʈ ����
	//	FRotator NewRot = GetActorRotation();
	//	NewRot.Yaw = RotYaw;
	//	SetActorRotation(NewRot);
	//}
}

void ANetActor::PrintNetLog()
{
	const FString conStr = GetNetConnection() != nullptr ? TEXT("Valid Connection") : TEXT("Invalid Connection");
	const FString ownerName = GetOwner() != nullptr ? GetOwner()->GetName() : TEXT("No Owner");
	const FString logStr = FString::Printf(TEXT("Connection : %s\nOwner Name : %s\nLocal Role : %s\nRemote Role : %s"), *conStr, *ownerName, *LOCAL_ROLE, *REMOTE_ROLE);
	DrawDebugString(GetWorld(), GetActorLocation() + FVector::UpVector * 100.f, logStr, nullptr, FColor::Red, 0, true, 1);

}

void ANetActor::FindOwner()
{
	if (HasAuthority()) {
		AActor* NewOwner = nullptr;
		float MinDist = SearchDistance;

		// �����������θ� ������! It++�� �ȵ�
		for (TActorIterator<ANetTPSCharacter> It(GetWorld()); It; ++It) {
			AActor* OtherActor = *It;
			float Dist = GetDistanceTo(OtherActor);

			if (Dist < MinDist) {
				MinDist = Dist;
				NewOwner = OtherActor;
			}
		}

		// Owner ���� -> ���ʴ� ���������� �ٲ� �� �ִ�!
		if (GetOwner() != NewOwner) {
			SetOwner(NewOwner);
		}
	}

	DrawDebugSphere(GetWorld(), GetActorLocation(), SearchDistance, 30, FColor::Yellow, false, 0, 0, 1);
}

void ANetActor::OnRep_RotYaw()
{
	// Ŭ���̾�Ʈ ����
	FRotator NewRot = GetActorRotation();
	NewRot.Yaw = RotYaw;
	SetActorRotation(NewRot);

	// ������Ʈ�� ����ð� ����
	lastTime = currentTime;

	// ����ð��� �ʱ�ȭ
	currentTime = 0.f;

}

// ���� ����ȭ
void ANetActor::OnRep_ChangeMatColor()
{
	if (Mat) {
		Mat->SetVectorParameterValue(TEXT("FloorColor"), MatColor);
	}
}

void ANetActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const 
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(ANetActor, RotYaw);
	DOREPLIFETIME(ANetActor, MatColor);
}