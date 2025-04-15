#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoginWidget.generated.h"


UCLASS()
class NETTPS_API ULoginWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	class UButton* btn_createRoom;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	class UEditableText* edit_roomName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	class USlider* slider_playerCount;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (BindWidget))
	class UTextBlock* txt_playerCount;

	UPROPERTY()
	class UNetGameInstance* GI;

public:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void CreateRoom();

	// Slider callback
	UFUNCTION()
	void OnValueChanged(float Value);


};
