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

	// 대역폭 조정
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

	// 서버일 경우
	if (HasAuthority()) {
		// 서버 영역
		AddActorLocalRotation(FRotator(0.f, 50.f * DeltaTime, 0.f));
		RotYaw = GetActorRotation().Yaw;
	}
	else {
		// 클라이언트 자체 보간
		// 경과시간 증가
		currentTime += DeltaTime;
		// 0으로 나눠지지 않도록 lastTime 값 체크
		if (lastTime < KINDA_SMALL_NUMBER) {
			return;
		}

		// 이전 경과시간과 현재 경과시간의 비율계산
		float lerpRatio = currentTime / lastTime;
		// 이전 경과시간만큼 회전할 것으로 새로운 회전값 계산
		float newYaw = RotYaw + 50 * lastTime;
		// 예측되는 값으로 진행된 시간만큼 보간처리
		float lerpYaw = FMath::Lerp(RotYaw, newYaw, lerpRatio);
		// 최종 적용
		FRotator CurRot = GetActorRotation();
		CurRot.Yaw = lerpYaw;
		SetActorRotation(CurRot);
	}

	//else {
	//	// 클라이언트 영역
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

		// 후위연산으로만 가능함! It++은 안됨
		for (TActorIterator<ANetTPSCharacter> It(GetWorld()); It; ++It) {
			AActor* OtherActor = *It;
			float Dist = GetDistanceTo(OtherActor);

			if (Dist < MinDist) {
				MinDist = Dist;
				NewOwner = OtherActor;
			}
		}

		// Owner 설정 -> 오너는 서버에서만 바꿀 수 있다!
		if (GetOwner() != NewOwner) {
			SetOwner(NewOwner);
		}
	}

	DrawDebugSphere(GetWorld(), GetActorLocation(), SearchDistance, 30, FColor::Yellow, false, 0, 0, 1);
}

void ANetActor::OnRep_RotYaw()
{
	// 클라이언트 영역
	FRotator NewRot = GetActorRotation();
	NewRot.Yaw = RotYaw;
	SetActorRotation(NewRot);

	// 업데이트된 경과시간 저장
	lastTime = currentTime;

	// 경과시간을 초기화
	currentTime = 0.f;

}

// 색상 동기화
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