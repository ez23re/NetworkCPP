#include "NetPlayerController.h"
#include "NetTPSGameMode.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SpectatorPawn.h"

void ANetPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority()) {
		gm = Cast<ANetTPSGameMode>(GetWorld()->GetAuthGameMode());
	}
}

void ANetPlayerController::ServerRPC_RespawnPlayer_Implementation()
{
	auto player = GetPawn();
	UnPossess();
	player->Destroy();
	
	gm->RestartPlayer(this);

}

void ANetPlayerController::ServerRPC_ChangeToSpectator_Implementation()
{
	// 관전자가 플레이어의 위치에 생성될 수 있도록 플레이어 정보를 가져온다
	APawn* player = GetPawn();
	if (player) {
		// 관전자를 생성
		FActorSpawnParameters params;
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		auto spectator = GetWorld()->SpawnActor<ASpectatorPawn>(gm->SpectatorClass, player->GetActorTransform(), params);
		// Possess하기
		Possess(spectator);
		// 이전 플레이어는 제거
		player->Destroy();
		
		// 5초 후 리스폰 시키기
		FTimerHandle handle;
		GetWorldTimerManager().SetTimer(handle, this, &ThisClass::ServerRPC_RespawnPlayer_Implementation, 5.f, false);
	}
}
