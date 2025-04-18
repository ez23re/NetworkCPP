#include "SessionSlotWidget.h"
#include "Components/TextBlock.h"
#include "NetGameInstance.h"
#include "Components/Button.h"

void USessionSlotWidget::NativeConstruct()
{	
	Super::NativeConstruct();

	btn_join->OnClicked.AddDynamic(this, &ThisClass::JoinSession);
}

void USessionSlotWidget::JoinSession()
{
	auto gi = Cast<UNetGameInstance>(GetWorld()->GetGameInstance());
	if (gi != nullptr) {
		gi->JoinSelectedSession(sessionNumber);
	}
}

void USessionSlotWidget::Set(const struct FSessionInfo& sessionInfo)
{
	txt_roomName->SetText(FText::FromString(sessionInfo.roomName));
	txt_hostName->SetText(FText::FromString(sessionInfo.hostName));
	txt_playerCount->SetText(FText::FromString(sessionInfo.playerCount));
	txt_pingSpeed->SetText(FText::FromString(FString::Printf(TEXT("%dms"), sessionInfo.pingSpeed)));

	sessionNumber = sessionInfo.index;
}

