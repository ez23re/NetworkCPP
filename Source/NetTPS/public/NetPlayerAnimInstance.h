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

	// 총쏘기에 사용할 몽타주
	UPROPERTY(EditDefaultsOnly, Category="Anim")
	class UAnimMontage* FireMontage;

	// 총쏘기 애니메이션 재생
	void PlayFireAnimation();

	// 회전값 기억변수
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MyAnimSettings")
	float PitchAngle;

	// 재장전 몽타주
	UPROPERTY(EditDefaultsOnly, Category="Anim")
	class UAnimMontage* ReloadMontage;

	// 재장전 애니메이션 재생
	void PlayReloadAnimation();

	// 재장전 애니메이션 노티파이이벤트
	UFUNCTION()
	void AnimNotify_OnReloadFinish();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MyAnimSettings")
	bool bIsDead = false;


};
