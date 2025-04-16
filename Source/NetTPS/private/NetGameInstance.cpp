#include "NetGameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "NetTPS.h"
#include "../../../../Plugins/Online/OnlineBase/Source/Public/Online/OnlineSessionNames.h"

void UNetGameInstance::Init()
{
	Super::Init();
	if (auto subsys = IOnlineSubsystem::Get()) {
		// ����ý������κ��� �����������̽� ��������
		sessionInterface = subsys->GetSessionInterface();
		sessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &UNetGameInstance::OnCreateSessionComplete);
		sessionInterface->OnFindSessionsCompleteDelegates.AddUObject(this, &UNetGameInstance::OnFindSessionsComplete);

		/*FTimerHandle handle;
		GetWorld()->GetTimerManager().SetTimer(handle, FTimerDelegate::CreateLambda([&]
			{
				CreateMySession(mySessionName, 10);
			}
			), 2.0f, false);*/
		FTimerHandle handle;
		GetWorld()->GetTimerManager().SetTimer(handle, FTimerDelegate::CreateLambda([&]
			{
				FindOtherSession();
			}
			), 2.0f, false);
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
	sessionSettings.Set(FName("ROOM_NAME"), StringBase64Encode(roomName), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// 8. ȣ��Ʈ ���� ����
	sessionSettings.Set(FName("HOST_NAME"), StringBase64Encode(mySessionName), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// netID
	FUniqueNetIdPtr netID = GetWorld()->GetFirstLocalPlayerFromController()->GetUniqueNetIdForPlatformUser().GetUniqueNetId();

	PRINTLOG(TEXT("Create Session Strat : %s"), *mySessionName);
	sessionInterface->CreateSession(*netID, FName(mySessionName), sessionSettings);
}

void UNetGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	PRINTLOG(TEXT("SessionName: %s, bWasSuccessful: %d"), *SessionName.ToString(), bWasSuccessful);
}

void UNetGameInstance::FindOtherSession()
{
	sessionSearch = MakeShareable(new FOnlineSessionSearch());
	// 1. ���� �˻� ���� ����
	sessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	// 2. Lan ���� : ������ �������� �Ǿ��ִ��� üũ
	sessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == FName("NULL");
	// 3. �ִ� �˻� ���� ��
	sessionSearch->MaxSearchResults = 10;
	// 4. ���� �˻�
	sessionInterface->FindSessions(0, sessionSearch.ToSharedRef());

}

void UNetGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	// ã�� ���н�
	if (!bWasSuccessful) {
		PRINTLOG(TEXT("Session search failed..."));
		return;
	}

	// ���� �˻���� �迭
	auto results = sessionSearch->SearchResults;
	PRINTLOG(TEXT("Search Result Count : %d"), results.Num());

	for (int i = 0;i < results.Num();++i) {
		auto sr = results[i];
		if (sr.IsValid() == false) {
			continue;
		}

		FString roomName;
		FString hostName;


		// ���� ���� ����ü ����
		FSessionInfo sessionInfo;
		sessionInfo.index = i;

		sr.Session.SessionSettings.Get(FName("ROOM_NAME"), roomName);
		sr.Session.SessionSettings.Get(FName("HOST_NAME"), hostName);
		sessionInfo.roomName = StringBase64Decode(roomName);
		sessionInfo.hostName = StringBase64Decode(hostName);

		//sr.Session.SessionSettings.Get(FName("ROOM_NAME"), sessionInfo.roomName);
		//sr.Session.SessionSettings.Get(FName("HOST_NAME"), sessionInfo.hostName);

		// ���� ������ �÷��̾��
		int32 maxPlayerCount = sr.Session.SessionSettings.NumPublicConnections; //public vs private?
		// ���� ������ �÷��̾� �� (�ִ�-���� ���� ������ ��)
		// NumOpenPublicConnenctions�� ���������� ���������� ���� ���´�
		int32 currentPlayerCount = maxPlayerCount - sr.Session.NumOpenPublicConnections;

		sessionInfo.playerCount = FString::Printf(TEXT("(%d/%d)"), currentPlayerCount, maxPlayerCount);

		// ping ���� (���ۿ����� 9999�� ���´�)
		sessionInfo.pingSpeed = sr.PingInMs;

		PRINTLOG(TEXT("%s"), *sessionInfo.ToString());
	}

	// ������ �����´�
	//for (auto sr : results) {
	//	// ������ ��ȿ���� üũ
	//	if (sr.IsValid() == false) {
	//		continue;
	//	}
	//	FString roomName;
	//	sr.Session.SessionSettings.Get(FName("ROOM_NAME"), roomName);
	//	FString hostName;
	//	sr.Session.SessionSettings.Get(FName("HOST_NAME"), hostName);
	//	// ��������(����) �̸�
	//	FString userName = sr.Session.OwningUserName;
	//	// ���� ������ �÷��̾��
	//	int32 maxPlayerCount = sr.Session.SessionSettings.NumPublicConnections; //public vs private?
	//	// ���� ������ �÷��̾� �� (�ִ�-���� ���� ������ ��)
	//	// NumOpenPublicConnenctions�� ���������� ���������� ���� ���´�
	//	int32 currentPlayerCount = maxPlayerCount - sr.Session.NumOpenPublicConnections;
	//	// ping ���� (���ۿ����� 9999�� ���´�)
	//	int32 pingSpeed = sr.PingInMs;

	//	PRINTLOG(TEXT("%s : %s(%s) - (%d/%d), %dms"), *roomName, *hostName, *userName, currentPlayerCount, maxPlayerCount, pingSpeed);
	//}
}

FString UNetGameInstance::StringBase64Encode(const FString& str)
{
	// Set�� �� : FString -> UTF8 (std::string) -> TArray<uint8> -> base64�� Encode
	std::string utf8String = TCHAR_TO_UTF8(*str);
	TArray<uint8> arrayData = TArray<uint8>((uint8*)(utf8String.c_str()), utf8String.length());
	return FBase64::Encode(arrayData);
}

FString UNetGameInstance::StringBase64Decode(const FString& str)
{
	// Get�� �� : FString -> base64�� Decode -> TArray<uint8> -> TCHAR
	TArray<uint8> arrayData;
	FBase64::Decode(str, arrayData);
	std::string utf8String((char*)arrayData.GetData(), arrayData.Num());
	return UTF8_TO_TCHAR(utf8String.c_str());
}

/*
* �𸮾��� FString�� �⺻ TCHAR �迭�� �Ǿ��ִ�
* TCHAR = UTF-16(wchar_t, 2byte)
* �ϴ� ���� ������ �̿��ϸ� ������
* ������ ��Ȯ�� �� �� ������ �Ƹ� UTF-8�� ����ϴ� �� ����
* �̷� ������ �ذ��ϱ� ���ؼ� Base64 ���ڵ� / ���ڵ��� �̿�
* �̰� �̿��ϴ� ������ �����ϰ� ��ȯ�� �ؼ� ������ ����
* Base64 ���ڵ� : ���ڿ��� uint8�� �迭�� ���� ��
* ASCII �ڵ�� ��ȯ�ؼ� ���
* 
* 2�� 6�� = 6bit �������� ���ڵ� = 6bit�� ��� ���ڵ�
* 
*/