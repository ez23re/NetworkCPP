#include "NetGameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "NetTPS.h"
#include "../../../../Plugins/Online/OnlineBase/Source/Public/Online/OnlineSessionNames.h"

void UNetGameInstance::Init()
{
	Super::Init();
	if (auto subsys = IOnlineSubsystem::Get()) {
		// 서브시스템으로부터 세션인터페이스 가져오기
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
	// 세션설정 변수
	FOnlineSessionSettings sessionSettings;

	// 1. Dedicated Server 접속 여부
	sessionSettings.bIsDedicated = false;

	// 2. 랜선(로컬)매칭을 할지 steam 매칭을 할지 여부
	FName subsysName = IOnlineSubsystem::Get()->GetSubsystemName();
	sessionSettings.bIsLANMatch = (subsysName == "NULL");

	// 3. 매칭이 온라인을 통해 노출될지 여부
	// false 이면 초대를 통해서만 입장이 가능
	// SendSessionInviteToFriend() 함수를 통해 친구를 초대할 수 있다.
	sessionSettings.bShouldAdvertise = true;

	// 4. 온라인 상태(Presence) 정보를 활용할지 여부
	sessionSettings.bUsesPresence = true;
	sessionSettings.bUseLobbiesIfAvailable = true;

	// 5. 게임진행중에 참여 허가할지 여부
	sessionSettings.bAllowJoinViaPresence = true;
	sessionSettings.bAllowJoinInProgress = true;

	// 6. 세션에 참여할 수 있는 공개(public) 연결의 최대 허용 수
	sessionSettings.NumPublicConnections = playerCount;

	// 7. 커스텀 룸네임 설정
	sessionSettings.Set(FName("ROOM_NAME"), StringBase64Encode(roomName), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	// 8. 호스트 네임 설정
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
	// 1. 세션 검색 조건 설정
	sessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
	// 2. Lan 여부 : 랜인지 스팀으로 되어있는지 체크
	sessionSearch->bIsLanQuery = IOnlineSubsystem::Get()->GetSubsystemName() == FName("NULL");
	// 3. 최대 검색 세션 수
	sessionSearch->MaxSearchResults = 10;
	// 4. 세션 검색
	sessionInterface->FindSessions(0, sessionSearch.ToSharedRef());

}

void UNetGameInstance::OnFindSessionsComplete(bool bWasSuccessful)
{
	// 찾기 실패시
	if (!bWasSuccessful) {
		PRINTLOG(TEXT("Session search failed..."));
		return;
	}

	// 세션 검색결과 배열
	auto results = sessionSearch->SearchResults;
	PRINTLOG(TEXT("Search Result Count : %d"), results.Num());

	for (int i = 0;i < results.Num();++i) {
		auto sr = results[i];
		if (sr.IsValid() == false) {
			continue;
		}

		FString roomName;
		FString hostName;


		// 세션 정보 구조체 선언
		FSessionInfo sessionInfo;
		sessionInfo.index = i;

		sr.Session.SessionSettings.Get(FName("ROOM_NAME"), roomName);
		sr.Session.SessionSettings.Get(FName("HOST_NAME"), hostName);
		sessionInfo.roomName = StringBase64Decode(roomName);
		sessionInfo.hostName = StringBase64Decode(hostName);

		//sr.Session.SessionSettings.Get(FName("ROOM_NAME"), sessionInfo.roomName);
		//sr.Session.SessionSettings.Get(FName("HOST_NAME"), sessionInfo.hostName);

		// 입장 가능한 플레이어수
		int32 maxPlayerCount = sr.Session.SessionSettings.NumPublicConnections; //public vs private?
		// 현재 입장한 플레이어 수 (최대-현재 입장 가능한 수)
		// NumOpenPublicConnenctions은 스팀에서만 정상적으로 값이 들어온다
		int32 currentPlayerCount = maxPlayerCount - sr.Session.NumOpenPublicConnections;

		sessionInfo.playerCount = FString::Printf(TEXT("(%d/%d)"), currentPlayerCount, maxPlayerCount);

		// ping 정보 (스템에서는 9999로 나온다)
		sessionInfo.pingSpeed = sr.PingInMs;

		PRINTLOG(TEXT("%s"), *sessionInfo.ToString());
	}

	// 정보를 가져온다
	//for (auto sr : results) {
	//	// 정보가 유효한지 체크
	//	if (sr.IsValid() == false) {
	//		continue;
	//	}
	//	FString roomName;
	//	sr.Session.SessionSettings.Get(FName("ROOM_NAME"), roomName);
	//	FString hostName;
	//	sr.Session.SessionSettings.Get(FName("HOST_NAME"), hostName);
	//	// 세션주인(방장) 이름
	//	FString userName = sr.Session.OwningUserName;
	//	// 입장 가능한 플레이어수
	//	int32 maxPlayerCount = sr.Session.SessionSettings.NumPublicConnections; //public vs private?
	//	// 현재 입장한 플레이어 수 (최대-현재 입장 가능한 수)
	//	// NumOpenPublicConnenctions은 스팀에서만 정상적으로 값이 들어온다
	//	int32 currentPlayerCount = maxPlayerCount - sr.Session.NumOpenPublicConnections;
	//	// ping 정보 (스템에서는 9999로 나온다)
	//	int32 pingSpeed = sr.PingInMs;

	//	PRINTLOG(TEXT("%s : %s(%s) - (%d/%d), %dms"), *roomName, *hostName, *userName, currentPlayerCount, maxPlayerCount, pingSpeed);
	//}
}

FString UNetGameInstance::StringBase64Encode(const FString& str)
{
	// Set할 때 : FString -> UTF8 (std::string) -> TArray<uint8> -> base64로 Encode
	std::string utf8String = TCHAR_TO_UTF8(*str);
	TArray<uint8> arrayData = TArray<uint8>((uint8*)(utf8String.c_str()), utf8String.length());
	return FBase64::Encode(arrayData);
}

FString UNetGameInstance::StringBase64Decode(const FString& str)
{
	// Get할 때 : FString -> base64로 Decode -> TArray<uint8> -> TCHAR
	TArray<uint8> arrayData;
	FBase64::Decode(str, arrayData);
	std::string utf8String((char*)arrayData.GetData(), arrayData.Num());
	return UTF8_TO_TCHAR(utf8String.c_str());
}

/*
* 언리얼의 FString은 기본 TCHAR 배열로 되어있다
* TCHAR = UTF-16(wchar_t, 2byte)
* 일단 스팀 서버를 이용하면 깨진다
* 원인을 정확히 알 수 없으나 아마 UTF-8을 사용하는 것 같다
* 이런 문제를 해결하기 위해서 Base64 인코딩 / 디코딩을 이용
* 이걸 이용하는 이유는 안전하게 변환을 해서 전달이 가능
* Base64 인코딩 : 문자열을 uint8로 배열로 만든 후
* ASCII 코드로 변환해서 사용
* 
* 2의 6승 = 6bit 형식으로 인코딩 = 6bit씩 끊어서 인코딩
* 
*/