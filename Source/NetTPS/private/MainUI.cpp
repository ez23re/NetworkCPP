#include "MainUI.h"
#include "Components/Image.h"
#include "Components/UniformGridPanel.h"

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
