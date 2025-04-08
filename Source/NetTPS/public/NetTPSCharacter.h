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

	// ���� �ڽ����� ���� ������Ʈ
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

	// �ʿ�Ӽ� : �� ��������
	UPROPERTY(Replicated)
	bool bHasPistol = false;

	// �������� ��
	UPROPERTY()
	AActor* OwnedPistol = nullptr;

	// �� �˻�����
	UPROPERTY(EditAnywhere, Category="Gun")
	float DistanceToGun = 200.f;

	// ���忡 ��ġ�� �ѵ�
	UPROPERTY()
	TArray<AActor*> PistolActors;

	void TakePistol(const FInputActionValue& Value);

	// ���� ������Ʈ�� ���̱�
	void AttachPistol(AActor* PistolActor);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* ReleaseAction;

	// �� ���� �Է� ó�� �Լ�
	void ReleasePistol(const FInputActionValue& Value);

	// ���� ������Ʈ���� �и�
	void DetachPistol(AActor* PistolActor);

	// �� ��� �Է� �׼�
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* FireAction;

	// �ѽ�� ó���Լ�
	void Fire(const FInputActionValue& Value);

	// �ǰ� particle
	UPROPERTY(EditDefaultsOnly, Category="Gun")
	class UParticleSystem* GunEffect;

	// ����� ���� Ŭ����
	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<class UMainUI> MainUIWidget;

	// MainUIWidget���κ��� ������� �ν��Ͻ� 
	UPROPERTY()
	class UMainUI* MainUI;

	// UI �ʱ�ȭ �Լ�
	void InitUIWidget();

	// �ִ� �Ѿ� ����
	UPROPERTY(EditAnywhere, Category="Bullet")
	int32 MaxBulletCount = 10;

	// ���� �Ѿ� ����
	int32 BulletCount = MaxBulletCount;

	// ���������� ����� InputAction
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Input")
	UInputAction* ReloadAction;

	// ������ �Է�ó�� �Լ�
	void ReloadPistol(const FInputActionValue& Value);

	// �Ѿ� UI �ʱ�ȭ �Լ�
	void InitAmmonUI();

	// ������ ������ ���
	bool bIsReloading = false;

	// Player ü��
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="HP")
	float MaxHP = 3;

	// ���� ü��
	UPROPERTY(BlueprintReadOnly, Category="HP")
	float hp = MaxHP;

	// getter, setter ����� 
	__declspec(property(get = GetHP, put = SetHP)) float HP;
	float GetHP();
	void SetHP(float value);
	
	UPROPERTY(VisibleAnywhere)
	class UWidgetComponent* HpUIComp;

	// �ǰ�ó��
	void DamageProcess();

	// �������
	bool bIsDead = false;



public:
	virtual void Tick(float DeltaSeconds) override;

	// ��Ʈ��ũ ���·α� ����Լ�
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

//------------------------------------------- MultiPlayer ��ҵ� -------------------------------------------//
public:
	// ����� RPC
	UFUNCTION(Server, Reliable)
	void ServerRPC_TakePistol();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_TakePistol(AActor* PistolActor);

	// �ѳ��� RPC
	UFUNCTION(Server, Reliable)
	void ServerRPC_ReleasePistol();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPC_ReleasePistol(AActor* PistolActor);

};

