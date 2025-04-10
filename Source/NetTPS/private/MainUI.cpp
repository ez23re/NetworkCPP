#include "MainUI.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "NetPlayerController.h"

void UMainUI::ShowCrosshair(bool bIsShow)
{
	if (bIsShow) {
		Img_Crosshair->SetVisibility(ESlateVisibility::Visible);
	}
	else {
		Img_Crosshair->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMainUI::AddBullet()
{
	auto BulletWidget = CreateWidget(GetWorld(), BulletUIFactory);
	BulletPanel->AddChildToUniformGrid(BulletWidget, 1, BulletPanel->GetChildrenCount());
}

void UMainUI::PopBullet(int32 Idx)
{
	BulletPanel->RemoveChildAt(Idx);
}

void UMainUI::RemoveAllAmmo()
{
	for (auto BulletWidget : BulletPanel->GetAllChildren()) {
		BulletPanel->RemoveChild(BulletWidget);
	}
}

void UMainUI::PlayDamageAnimation()
{
	PlayAnimation(DamageAnim);
}

void UMainUI::NativeConstruct()
{
	Super::NativeConstruct();

	btn_retry->OnClicked.AddDynamic(this, &UMainUI::OnRetry);
}

void UMainUI::OnRetry()
{
	// �������� UI �Ⱥ��̵��� ó��
	GameOverUI->SetVisibility(ESlateVisibility::Hidden);

	auto pc = Cast<ANetPlayerController>(GetWorld()->GetFirstPlayerController());
	if (pc) {
		// ���콺 Ŀ�� �Ⱥ��̵��� ó��
		pc->SetShowMouseCursor(false);
		pc->ServerRPC_RespawnPlayer();
	}
}
