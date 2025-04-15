#include "LoginWidget.h"
#include "NetGameInstance.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/EditableText.h"
#include "Components/TextBlock.h"

void ULoginWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GI = Cast<UNetGameInstance>(GetWorld()->GetGameInstance());
	btn_createRoom->OnClicked.AddDynamic(this, &ULoginWidget::CreateRoom);
	slider_playerCount->OnValueChanged.AddDynamic(this, &ULoginWidget::OnValueChanged);
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
