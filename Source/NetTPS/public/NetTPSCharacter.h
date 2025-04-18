#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "NetTPSCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

UCLASS(config=Game)
class ANetTPSCharacter : public ACharacter
{
	GENERATED_BODY()

	// 총을 자식으로 붙일 컴포넌트
	UPROPERTY(VisibleAnywhere)
	class USceneComponent* GunComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* TakePistolAction;

	// 필요속성 : 총 소유여부
	UPROPERTY(Replicated)
	bool bHasPistol = false;

	// 소유중인 총
	UPROPERTY()
	AActor* OwnedPistol = nullptr;

	// 총 검색범위
	UPROPERTY(EditAnywhere, Category="Gun")
	float DistanceToGun = 200.f;

	// 월드에 배치된 총들
	UPROPERTY()
	TArray<AActor*> PistolActors;

	void TakePistol(const FInputActionValue& Value);

	// 총을 컴포넌트에 붙이기
	void AttachPistol(AActor* pistolActor);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* ReleaseAction;

	// 총 놓기 입력 처리 함수
	void ReleasePistol(const FInputActionValue& Value);

	// 총을 컴포넌트에서 분리
	void DetachPistol(AActor* pistolActor);

	// 총 쏘기 입력 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* FireAction;

	// 총쏘기 처리함수
	void Fire(const FInputActionValue& Value);

	// 피격 particle
	UPROPERTY(EditDefaultsOnly, Category="Gun")
	class UParticleSystem* GunEffect;

	// 사용할 위젯 클래스
	//UPROPERTY(EditDefaultsOnly, Category="UI")
	//TSubclassOf<class UMainUI> MainUIWidget;

	// MainUIWidget으로부터 만들어진 인스턴스 
	UPROPERTY()
	class UMainUI* MainUI;

	// UI 초기화 함수
	void InitUIWidget();

	// 최대 총알 개수
	UPROPERTY(EditAnywhere, Category="Bullet")
	int32 MaxBulletCount = 10;

	// 남은 총알 개수
	UPROPERTY(Replicated)
	int32 BulletCount = MaxBulletCount;

	// 재장전에서 사용할 InputAction
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* ReloadAction;

	// 재장전 입력처리 함수
	void ReloadPistol(const FInputActionValue& Value);

	// 총알 UI 초기화 함수
	void InitAmmonUI();

	// 재장전 중인지 기억
	bool bIsReloading = false;

	// Player 체력
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HP")
	float MaxHP = 3;

	// 현재 체력
	UPROPERTY(ReplicatedUsing=OnRep_HP, BlueprintReadOnly, Category="HP")
	float hp = MaxHP;
	UFUNCTION()
	void OnRep_HP();

	// getter, setter 만드는 
	__declspec(property(get = GetHP, put = SetHP)) float HP;
	float GetHP();
	void SetHP(float value);
	
	UPROPERTY(VisibleAnywhere)
	class UWidgetComponent* HpUIComp;

	// 피격처리
	void DamageProcess();

	// 사망여부
	bool bIsDead = false;

	// 카메라 셰이크
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<class UCameraShakeBase> DamageCameraShake;

	// 사망처리
	void DieProcess();

	// 이 함수는 서버에서만 호출 된다
	virtual void PossessedBy(AController* NewController) override;
	

public:
	virtual void Tick(float DeltaSeconds) override;

	// 네트워크 상태로그 출력함수
	void PrintNetLog();



public:
	ANetTPSCharacter();

	virtual void BeginPlay() override;
	

protected:

	void Move(const FInputActionValue& Value);

	void Look(const FInputActionValue& Value);
			

protected:

	virtual void NotifyControllerChanged() override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

//------------------------------------------- MultiPlayer 요소들 -------------------------------------------//
public:
	// 총잡기 RPC
	UFUNCTION(Server, Reliable)
	void ServerRPC_TakePistol();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_TakePistol(AActor* pistolActor);

	// 총놓기 RPC
	UFUNCTION(Server, Reliable)
	void ServerRPC_ReleasePistol();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_ReleasePistol(AActor* pistolActor);


public:
	// 총쏘기 RPC
	UFUNCTION(Server, Reliable)
	void ServerRPC_Fire();
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPC_Fire(bool bHit, const FHitResult& HitInfo);
	
	
public:
	// 재장전 RPC
	UFUNCTION(Server, Reliable)
	void ServerRPC_Reload();
	UFUNCTION(Client, Reliable)
	void ClientRPC_Reload();



};

