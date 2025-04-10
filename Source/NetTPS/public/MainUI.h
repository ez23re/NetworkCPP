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

	// Crosshair ON/OFF 처리
	void ShowCrosshair(bool bIsShow);


	// 총알 위젯이 추가될 패널
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UUniformGridPanel* BulletPanel;

	// 총알 위젯 클래스
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Bullet")
	TSubclassOf<class UUserWidget> BulletUIFactory;

	// 총알 위젯 추가함수
	void AddBullet();

	// 총알 제거함수
	void PopBullet(int32 Idx);

	// 모든 총알UI 제거
	void RemoveAllAmmo();


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="HP")
	float HP = 1.f;


	// DamageUI 애니메이션 
	UPROPERTY(EditDefaultsOnly, meta=(BindWidgetAnim), Transient, Category="MySettings")
	class UWidgetAnimation* DamageAnim;
	// 피격처리 애니메이션 재생
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
