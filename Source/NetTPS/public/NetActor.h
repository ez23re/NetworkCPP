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

	// ��Ʈ��ũ ���·α� ����Լ�
	void PrintNetLog();

	// Owner ���⿵��
	UPROPERTY(EditAnywhere)
	float SearchDistance = 200.f;

	// Owner ����
	void FindOwner();

	// ȸ���� ����ȭ ����
	//UPROPERTY(Replicated)
	UPROPERTY(ReplicatedUsing=OnRep_RotYaw)
	float RotYaw = 0.f;
	
	UFUNCTION()
	void OnRep_RotYaw();

	float currentTime = 0.f;
	float lastTime = 0.f;

	UPROPERTY()
	class UMaterialInstanceDynamic* Mat;
	// ������ ����ȭ�� ����
	UPROPERTY(ReplicatedUsing=OnRep_ChangeMatColor)
	FLinearColor MatColor;
	UFUNCTION()
	void OnRep_ChangeMatColor();


};
