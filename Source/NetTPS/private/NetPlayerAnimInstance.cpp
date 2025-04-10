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

	// �̵����� ���� ���ְ� ����
	if (player) {
		speed = FVector::DotProduct(player->GetVelocity(), player->GetActorForwardVector());
		direction = FVector::DotProduct(player->GetVelocity(), player->GetActorRightVector());
		
		// ȸ���� ����
		PitchAngle = -player->GetBaseAimRotation().GetNormalized().Pitch; // ��ȣ �ݴ��
		PitchAngle = FMathf::Clamp(PitchAngle, -60.f, 60.f); // Fmath�� ����

		// �� ���� ���� ����
		bHasPistol = player->bHasPistol;

		// ������� ����
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
		player->DieProcess(); // ��Ʈ���ϰ� �ִ� ���� ȭ�鿡����
	}
}
