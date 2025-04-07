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

	// Crosshair ON/OFF Ã³¸®
	void ShowCrosshair(bool bIsShow);


	// ÃÑ¾Ë À§Á¬ÀÌ Ãß°¡µÉ ÆÐ³Î
	UPROPERTY(BlueprintReadWrite, meta=(BindWidget))
	class UUniformGridPanel* BulletPanel;

	// ÃÑ¾Ë À§Á¬ Å¬·¡½º
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Bullet")
	TSubclassOf<class UUserWidget> BulletUIFactory;

	// ÃÑ¾Ë À§Á¬ Ãß°¡ÇÔ¼ö
	void AddBullet();

	// ÃÑ¾Ë Á¦°ÅÇÔ¼ö
	void PopBullet(int32 Idx);

	// ¸ðµç ÃÑ¾ËUI Á¦°Å
	void RemoveAllAmmo();


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="HP")
	float HP = 1.f;

};
