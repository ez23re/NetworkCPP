#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "NetActor.generated.h"

UCLASS()
class NETTPS_API ANetActor : public AActor
{
	GENERATED_BODY()
	
public:	
	ANetActor();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

public:
	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* MeshComp;

	// 네트워크 상태로그 출력함수
	void PrintNetLog();

	// Owner 검출영역
	UPROPERTY(EditAnywhere)
	float SearchDistance = 200.f;

	// Owner 설정
	void FindOwner();

	// 회전값 동기화 변수
	//UPROPERTY(Replicated)
	UPROPERTY(ReplicatedUsing=OnRep_RotYaw)
	float RotYaw = 0.f;
	
	UFUNCTION()
	void OnRep_RotYaw();

	float currentTime = 0.f;
	float lastTime = 0.f;

	UPROPERTY()
	class UMaterialInstanceDynamic* Mat;
	// 재질에 동기화될 색상
	UPROPERTY(ReplicatedUsing=OnRep_ChangeMatColor)
	FLinearColor MatColor;
	UFUNCTION()
	void OnRep_ChangeMatColor();


};
