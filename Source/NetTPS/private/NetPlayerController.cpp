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
	// �����ڰ� �÷��̾��� ��ġ�� ������ �� �ֵ��� �÷��̾� ������ �����´�
	APawn* player = GetPawn();
	if (player) {
		// �����ڸ� ����
		FActorSpawnParameters params;
		params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		auto spectator = GetWorld()->SpawnActor<ASpectatorPawn>(gm->SpectatorClass, player->GetActorTransform(), params);
		// Possess�ϱ�
		Possess(spectator);
		// ���� �÷��̾�� ����
		player->Destroy();
		
		// 5�� �� ������ ��Ű��
		FTimerHandle handle;
		GetWorldTimerManager().SetTimer(handle, this, &ThisClass::ServerRPC_RespawnPlayer_Implementation, 5.f, false);
	}
}
