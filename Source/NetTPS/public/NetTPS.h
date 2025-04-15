#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(NetLog, Log, All);

#define LOCAL_ROLE (UEnum::GetValueAsString<ENetRole>(GetLocalRole()))
#define REMOTE_ROLE (UEnum::GetValueAsString<ENetRole>(GetRemoteRole()))

#define NETMODE (GetWorld()->GetNetMode()==ENetMode::NM_Client ? TEXT("Client") : GetWorld()->GetNetMode()==NM_Standalone ? TEXT("Standalone") : TEXT("Server"))
#define CALLINFO (FString(__FUNCTION__) + TEXT("(") + FString::FromInt(__LINE__) + TEXT(")") )
#define PRINTLOG( fmt, ...) UE_LOG(NetLog, Warning, TEXT("[%s]%s : %s"), NETMODE, *CALLINFO, *FString::Printf(fmt, ##__VA_ARGS__))
