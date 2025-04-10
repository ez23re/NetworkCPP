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

	// ����� ����Ŭ����
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<class UMainUI> MainUIWidget;
	// MainUIWidget���κ��� ������� �ν��Ͻ�
	UPROPERTY()
	class UMainUI* MainUI;


};
