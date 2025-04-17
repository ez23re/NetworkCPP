#include "LoginWidget.h"
#include "NetGameInstance.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "SessionSlotWidget.h"
#include "Components/ScrollBox.h"

void ULoginWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GI = Cast<UNetGameInstance>(GetWorld()->GetGameInstance());
	GI->onSearchCompleted.AddDynamic(this, &ThisClass::AddSlotWidget);
	GI->onSearchState.AddDynamic(this, &ThisClass::OnChangeButtonEnable);



	btn_createRoom->OnClicked.AddDynamic(this, &ULoginWidget::CreateRoom);
	slider_playerCount->OnValueChanged.AddDynamic(this, &ULoginWidget::OnValueChanged);

	btn_createSession->OnClicked.AddDynamic(this, &ThisClass::SwitchCreatePanel);
	btn_findSession->OnClicked.AddDynamic(this, &ThisClass::SwitchFindPanel);

	btn_back->OnClicked.AddDynamic(this, &ThisClass::BackToMain);
	btn_back_1->OnClicked.AddDynamic(this, &ThisClass::BackToMain);

	btn_find->OnClicked.AddDynamic(this, &ThisClass::OnClickedFindSession);
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
	OnClickedFindSession();
}

void ULoginWidget::BackToMain()
{
	WidgetSwitcher->SetActiveWidgetIndex(0);
}

void ULoginWidget::OnClickedFindSession()
{
	// 기존 슬롯이 있다면 모두 지운다
	scroll_roomList->ClearChildren();
	if (GI != nullptr) {
		GI->FindOtherSession();
	}
}

void ULoginWidget::OnChangeButtonEnable(bool bIsSearching)
{
	btn_find->SetIsEnabled(!bIsSearching);
	
	if (bIsSearching) {
		// 검색중 보이도록 처리
		txt_findingMsg->SetVisibility(ESlateVisibility::Visible);
	}
	else {
		// 검색중 사라지도록 처리
		txt_findingMsg->SetVisibility(ESlateVisibility::Hidden);
	}
}

void ULoginWidget::AddSlotWidget(const struct FSessionInfo& sessionInfo)
{
	auto slot = CreateWidget<USessionSlotWidget>(this, sessionInfoWidget);
	slot->Set(sessionInfo);
	scroll_roomList->AddChild(slot);
}
