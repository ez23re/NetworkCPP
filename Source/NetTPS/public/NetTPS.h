#pragma once

#include "CoreMinimal.h"

#define LOCAL_ROLE (UEnum::GetValueAsString<ENetRole>(GetLocalRole()))
#define REMOTE_ROLE (UEnum::GetValueAsString<ENetRole>(GetRemoteRole()))