#include "NetPlayerAnimInstance.h"
#include "NetTPSCharacter.h"
#include "Animation/AnimMontage.h"
#include "MathUtil.h"

void UNetPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	player = Cast<ANetTPSCharacter>(TryGetPawnOwner());
}

void UNetPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	// 이동방향 결정 해주고 있음
	if (player) {
		speed = FVector::DotProduct(player->GetVelocity(), player->GetActorForwardVector());
		direction = FVector::DotProduct(player->GetVelocity(), player->GetActorRightVector());
		
		// 회전값 적용
		PitchAngle = -player->GetBaseAimRotation().GetNormalized().Pitch; // 부호 반대로
		PitchAngle = FMathf::Clamp(PitchAngle, -60.f, 60.f); // Fmath랑 차이

		// 총 소유 여부 적용
		bHasPistol = player->bHasPistol;

		// 사망여부 적용
		bIsDead = player->bIsDead;
	}
}

void UNetPlayerAnimInstance::PlayFireAnimation()
{
	if (bHasPistol && FireMontage) {
		Montage_Play(FireMontage);
	}
}

void UNetPlayerAnimInstance::PlayReloadAnimation()
{
	if (bHasPistol && ReloadMontage) {
		Montage_Play(ReloadMontage);
	}
}

void UNetPlayerAnimInstance::AnimNotify_OnReloadFinish()
{
	player->InitAmmonUI();

}

void UNetPlayerAnimInstance::AnimNotify_DieEnd()
{
	if (player && player->IsLocallyControlled()) {
		player->DieProcess(); // 컨트롤하고 있는 유저 화면에서만
	}
}
