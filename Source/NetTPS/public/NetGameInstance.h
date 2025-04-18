#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "OnlineSessionSettings.h"
#include "../../../../Plugins/Online/OnlineSubsystem/Source/Public/Interfaces/OnlineSessionInterface.h"
#include "NetGameInstance.generated.h"

USTRUCT(BlueprintType)
struct FSessionInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString roomName;
	UPROPERTY(BlueprintReadOnly)
	FString hostName;
	UPROPERTY(BlueprintReadOnly)
	FString playerCount;
	UPROPERTY(BlueprintReadOnly)
	int32 pingSpeed;
	UPROPERTY(BlueprintReadOnly)
	int32 index;

	inline FString ToString() 
	{
		return FString::Printf(TEXT("[%d] %s : %s - %s, %dms"), index, *roomName, *hostName, *playerCount, pingSpeed);
	}
};

// ���� �˻��� ������ �� ȣ��� ��������Ʈ
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSearchSignature, const FSessionInfo&, sessionInfo);

// ���� �˻� ���� ��������Ʈ
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSearchStateSignature, bool, bIsSearching);


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
	FString mySessionName = "  ";

	UFUNCTION()
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);

	// �� �˻�
	TSharedPtr<FOnlineSessionSearch> sessionSearch;
	void FindOtherSession();

	void OnFindSessionsComplete(bool bWasSuccessful);

	// �� ã�� �Ϸ� �ݹ��� ����� ��������Ʈ
	FSearchSignature onSearchCompleted;
	// �� ã�� ���� �ݹ� ��������Ʈ
	FSearchStateSignature onSearchState;


	// ����(��) ����
	void JoinSelectedSession(int32 index);
	// ���� ���� �ݹ�
	void OnJoinSessionCompleted(FName sessionName, EOnJoinSessionCompleteResult::Type result);


public:
	// �ٱ��� ���ڵ�
	FString StringBase64Encode(const FString& str);
	FString StringBase64Decode(const FString& str);
};
