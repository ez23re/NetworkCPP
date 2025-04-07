#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "NetPlayerAnimInstance.generated.h"


UCLASS()
class NETTPS_API UNetPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MyAnimSettings")
	bool bHasPistol = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MyAnimSettings")
	float direction;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MyAnimSettings")
	float speed;

	UPROPERTY()
	class ANetTPSCharacter* player;

	// �ѽ�⿡ ����� ��Ÿ��
	UPROPERTY(EditDefaultsOnly, Category="Anim")
	class UAnimMontage* FireMontage;

	// �ѽ�� �ִϸ��̼� ���
	void PlayFireAnimation();

	// ȸ���� ��ﺯ��
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MyAnimSettings")
	float PitchAngle;

	// ������ ��Ÿ��
	UPROPERTY(EditDefaultsOnly, Category="Anim")
	class UAnimMontage* ReloadMontage;

	// ������ �ִϸ��̼� ���
	void PlayReloadAnimation();

	// ������ �ִϸ��̼� ��Ƽ�����̺�Ʈ
	UFUNCTION()
	void AnimNotify_OnReloadFinish();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MyAnimSettings")
	bool bIsDead = false;


};
