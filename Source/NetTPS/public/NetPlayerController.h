#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NetPlayerController.generated.h"

UCLASS()
class NETTPS_API ANetPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	class ANetTPSGameMode* gm;

public:
	virtual void BeginPlay() override;
	
	UFUNCTION(Server, Reliable)
	void ServerRPC_RespawnPlayer();

	// 사용할 위젯클래스
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<class UMainUI> MainUIWidget;
	// MainUIWidget으로부터 만들어진 인스턴스
	UPROPERTY()
	class UMainUI* MainUI;


};
