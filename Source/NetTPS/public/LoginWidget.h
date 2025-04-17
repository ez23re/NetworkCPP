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


	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UWidgetSwitcher* WidgetSwitcher;
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UButton* btn_createSession;
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UButton* btn_findSession;


	UFUNCTION()
	void SwitchCreatePanel();
	UFUNCTION()
	void SwitchFindPanel();

	// ����ȭ�� ���ư��� ��ư
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UButton* btn_back;
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UButton* btn_back_1;
	UFUNCTION()
	void BackToMain();

	// �� �˻� ��ư
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UButton* btn_find;

	// �˻� �� �޽��� 
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UTextBlock* txt_findingMsg;


	// �� ã�� ��ư Ŭ���� ȣ��� �ݹ�
	UFUNCTION()
	void OnClickedFindSession();

	// �� ã�� ���� �̺�Ʈ �ݹ�
	UFUNCTION()
	void OnChangeButtonEnable(bool bIsSearching);


	// ���ǽ���
	// CanVas_FindRoom�� ��ũ�ѹڽ� ����
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	class UScrollBox* scroll_roomList;
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<class USessionSlotWidget> sessionInfoWidget;
	UFUNCTION()
	void AddSlotWidget(const struct FSessionInfo& sessionInfo);


};
