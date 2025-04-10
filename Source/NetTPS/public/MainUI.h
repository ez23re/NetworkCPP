#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainUI.generated.h"


UCLASS()
class NETTPS_API UMainUI : public UUserWidget
{
	GENERATED_BODY()
	

public:
	UPROPERTY(BlueprintReadWrite, Category="UI", meta=(BindWidget))
	class UImage* Img_Crosshair;

	// Crosshair ON/OFF ó��
	void ShowCrosshair(bool bIsShow);


	// �Ѿ� ������ �߰��� �г�
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UUniformGridPanel* BulletPanel;

	// �Ѿ� ���� Ŭ����
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Bullet")
	TSubclassOf<class UUserWidget> BulletUIFactory;

	// �Ѿ� ���� �߰��Լ�
	void AddBullet();

	// �Ѿ� �����Լ�
	void PopBullet(int32 Idx);

	// ��� �Ѿ�UI ����
	void RemoveAllAmmo();


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="HP")
	float HP = 1.f;


	// DamageUI �ִϸ��̼� 
	UPROPERTY(EditDefaultsOnly, meta=(BindWidgetAnim), Transient, Category="MySettings")
	class UWidgetAnimation* DamageAnim;
	// �ǰ�ó�� �ִϸ��̼� ���
	void PlayDamageAnimation();

	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UHorizontalBox* GameOverUI;
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UButton* btn_retry;	
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UButton* btn_exit;
	
public:
	virtual void NativeConstruct() override;
	UFUNCTION()
	void OnRetry();
};
