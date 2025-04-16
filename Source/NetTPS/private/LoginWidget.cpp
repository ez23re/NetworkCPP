#include "LoginWidget.h"
#include "NetGameInstance.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"

void ULoginWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GI = Cast<UNetGameInstance>(GetWorld()->GetGameInstance());
	btn_createRoom->OnClicked.AddDynamic(this, &ULoginWidget::CreateRoom);
	slider_playerCount->OnValueChanged.AddDynamic(this, &ULoginWidget::OnValueChanged);

	btn_createSession->OnClicked.AddDynamic(this, &ThisClass::SwitchCreatePanel);
	btn_findSession->OnClicked.AddDynamic(this, &ThisClass::SwitchFindPanel);

	btn_back->OnClicked.AddDynamic(this, &ThisClass::BackToMain);
	btn_back_1->OnClicked.AddDynamic(this, &ThisClass::BackToMain);
}

void ULoginWidget::CreateRoom()
{
	if (!GI || edit_roomName->GetText().IsEmpty()) return;

	GI->CreateMySession(edit_roomName->GetText().ToString(), slider_playerCount->GetValue());
}

void ULoginWidget::OnValueChanged(float Value)
{
	txt_playerCount->SetText(FText::AsNumber(Value));
}

void ULoginWidget::SwitchCreatePanel()
{
	WidgetSwitcher->SetActiveWidgetIndex(1);
}

void ULoginWidget::SwitchFindPanel()
{
	WidgetSwitcher->SetActiveWidgetIndex(2);
}

void ULoginWidget::BackToMain()
{
	WidgetSwitcher->SetActiveWidgetIndex(0);
}
