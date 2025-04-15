#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSessionSettings.h"
#include "NetGameInstance.generated.h"

UCLASS()
class NETTPS_API UNetGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

public:
	IOnlineSessionPtr sessionInterface;

	void CreateMySession(FString roomName, int32 playerCount);

	// ���� ȣ��Ʈ �̸�
	FString mySessionName = "LEESEJIN";

	UFUNCTION()
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

};
