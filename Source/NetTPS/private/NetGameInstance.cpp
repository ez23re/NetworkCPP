#include "NetGameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "NetTPS.h"

void UNetGameInstance::Init()
{
	Super::Init();
	if (auto subsys = IOnlineSubsystem::Get()) {
		sessionInterface = subsys->GetSessionInterface();
		sessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnCreateSessionComplete);

		/*FTimerHandle handle;
		GetWorld()->GetTimerManager().SetTimer(handle, FTimerDelegate::CreateLambda([&]
			{
				CreateMySession(mySessionName, 10);

			}
		), 2.0f, false);*/
	}
}

void UNetGameInstance::CreateMySession(FString roomName, int32 playerCount)
{
	// ���Ǽ��� ����
	FOnlineSessionSettings sessionSettings;

	// 1. Dedicated Server ���� ����
	sessionSettings.bIsDedicated = false;

	// 2. ����(����)��Ī�� ���� steam ��Ī�� ���� ����
	FName subsysName = IOnlineSubsystem::Get()->GetSubsystemName();
	sessionSettings.bIsLANMatch = (subsysName == "NULL");

	// 3. ��Ī�� �¶����� ���� ������� ����
	// false �̸� �ʴ븦 ���ؼ��� ������ ����
	// SendSessionInviteToFriend() �Լ��� ���� ģ���� �ʴ��� �� �ִ�.
	sessionSettings.bShouldAdvertise = true;

	// 4. �¶��� ����(Presence) ������ Ȱ������ ����
	sessionSettings.bUsesPresence = true;
	sessionSettings.bUseLobbiesIfAvailable = true;

	// 5. ���������߿� ���� �㰡���� ����
	sessionSettings.bAllowJoinViaPresence = true;
	sessionSettings.bAllowJoinInProgress = true;

	// 6. ���ǿ� ������ �� �ִ� ����(public) ������ �ִ� ��� ��
	sessionSettings.NumPublicConnections = playerCount;

	// 7. Ŀ���� ����� ����
	sessionSettings.Set(FName("ROOM_NAME"), roomName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// 8. ȣ��Ʈ ���� ����
	sessionSettings.Set(FName("HOST_NAME"), mySessionName, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// netID
	FUniqueNetIdPtr netID = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();

	PRINTLOG(TEXT("Create Session Strat : %s"), *mySessionName);
	sessionInterface->CreateSession(*netID, FName(mySessionName), sessionSettings);
}

void UNetGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	PRINTLOG(TEXT("SessionName: %s, bWasSuccessful: %d"), *SessionName.ToString(), bWasSuccessful);
}
