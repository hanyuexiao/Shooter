// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Component/ActorComponent/CombatComponent.h"
#include "Character/BlasterTypes/TurningInPlace.h"
#include "Components/TimelineComponent.h"
#include "Interfaces/InteractWithCrosshairsInterface.h"
#include "BlasterTypes/CombatState.h"
#include "WS_Player.generated.h"

class ULagCompensationComponent;
class UBuffComponent;
class UCameraComponent;
struct FInputActionValue;
class UInputAction;
class UInputMappingContext;
class AWS_PlayerControll;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLeftGame);

UCLASS()
class MENUSYSTEM_API AWS_Player : public ACharacter, public IInteractWithCrosshairsInterface
{
	GENERATED_BODY()

public:
	AWS_Player();

	virtual void Tick(float DeltaTime) override;
	
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostInitializeComponents() override;

	TObjectPtr<UInputMappingContext> GetDefaultMappingContext() const {return DefaultIMC;}

	bool IsWeaponEquipped();

	bool IsCrouched() const {return bIsCrouched;}

	bool IsAiming() const {return CombatComponent->bAiming;}

	FORCEINLINE float GetAoYaw() const {return AO_Yaw;}
	
	FORCEINLINE float GetAoPitch() const {return AO_Pitch;}

	FORCEINLINE ETurningInPlace GetTurningInPlace() const {return TurningInPlace;}
	
	FORCEINLINE bool ShouldRotateRootBone() const { return bRotateRootBone; }

	FORCEINLINE float GetHealth() const { return Health; }

	FORCEINLINE void SetHealth(float Amount) { Health = Amount; }

	FORCEINLINE float GetMaxHealth() const { return MaxHealth; }
	
	FORCEINLINE float GetShield() const { return Shield; }
	
	FORCEINLINE void SetShield(float Amount) { Shield = Amount; }
	
	FORCEINLINE float GetMaxShield() const { return MaxShield; }
	
	FORCEINLINE UCombatComponent* GetCombat() const { return CombatComponent; }
	
	FORCEINLINE bool GetDisableGameplay() const { return bDisableGameplay; }

	FORCEINLINE UAnimMontage* GetReloadMontage() const { return ReloadMontage; }

	FORCEINLINE UStaticMeshComponent* GetAttachedGrenade() const { return AttachedGrenade; }

	FORCEINLINE UBuffComponent* GetBuff() const { return Buff; }

	FORCEINLINE ULagCompensationComponent* GetLagCompensation() const { return LagCompensation; }

	bool IsLocallyReloading();
	
	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScopeWidget(bool bShowScope);
	
	FORCEINLINE const bool IsAiming();
	
	AWS_Weapon* GetEquippedWeapon();

	/** 
	* Play montages
	*/
	
	void PlayFireMontage(bool bAiming);

	void PlayElimMontage();

	void PlayReloadMontage();

	void PlayThrowGrenadeMontage();

	void PlaySwapMontage();
	
	virtual void Destroyed() override;

	virtual void OnRep_ReplicatedMovement() override;
	
	void SetOverlappingWeapon(AWS_Weapon* Weapon);

	FVector GetHitTarget() const;

	FORCEINLINE UCameraComponent* GetFollowCamera() const { return FollowCamera; }

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);
	
	void Elim(bool bPlayerLeftGame);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastElim(bool bPlayerLeftGame);
	
	FORCEINLINE bool IsElimmed() const { return bElimmed; }

	ECombatState GetCombatState() const;

	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	void UpdateHUDHealth();
	void UpdateHUDShield();
	void UpdateHUDAmmo();

	void SpawnDefaultWeapon();

	UPROPERTY()
	TMap<FName, class UBoxComponent*> HitCollisionBoxes;

	bool bFinishedSwapping = false;

	UFUNCTION(Server, Reliable)
	void ServerLeaveGame();

	FOnLeftGame OnLeftGame;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastGainedTheLead();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLostTheLead();
	
protected:
	virtual void BeginPlay() override;

	void MoveForward(const FInputActionValue& Value);
	void MoveRight(const FInputActionValue& Value);
	void Turn(const FInputActionValue& Value);
	void LookUp(const FInputActionValue& Value);
	void Jump(const FInputActionValue& Value);
	void EquipButtonPressed();
	void CrouchButtonPressed();
	void ReloadButtonPressed();
	void AimButtonPressed();
	void AimButtonRelease();
	void GrenadeButtonPressed();
	void AimOffset(float DeltaTime);
	void CalculateAO_Pitch();
	void SimProxiesTurn();
	void TurnInPlace(float DeltaTime);
	void FireButtonPressed();
	void FireButtonReleased();
	void PlayHitReactMontage();
	void RotateInPlace(float DeltaTime);

	/** 
	* Hit boxes used for server-side rewind
	*/

	UPROPERTY(EditAnywhere)
	class UBoxComponent* head;

	UPROPERTY(EditAnywhere)
	UBoxComponent* pelvis;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_02;

	UPROPERTY(EditAnywhere)
	UBoxComponent* spine_03;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* upperarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* lowerarm_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* hand_r;
	
	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* thigh_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* calf_r;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_l;

	UPROPERTY(EditAnywhere)
	UBoxComponent* foot_r;
	
	UFUNCTION(Server, Reliable)
	void Server_EquipButtonPressed();
	void DropOrDestroyWeapon(AWS_Weapon* Weapon);
	void DropOrDestroyWeapons();
	
	UPROPERTY(EditAnywhere,Category = "MovingControl")
	TObjectPtr<UInputMappingContext> DefaultIMC;

	UPROPERTY(EditDefaultsOnly,Category = "MovingControl")
	TObjectPtr<UInputAction> MoveForwardAction;

	UPROPERTY(EditDefaultsOnly,Category = "MovingControl")
	TObjectPtr<UInputAction> MoveRightAction;
   	
	UPROPERTY(EditDefaultsOnly,Category = "MovingControl")
	TObjectPtr<UInputAction> TurnAction;
   	
	UPROPERTY(EditDefaultsOnly,Category = "MovingControl")
	TObjectPtr<UInputAction> LookUpAction;

	UPROPERTY(EditDefaultsOnly,Category = "MovingControl")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly,Category = "Pickup")
	TObjectPtr<UInputAction> EquipWeaponAction;

	UPROPERTY(EditDefaultsOnly,Category = "MovingControl")
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditDefaultsOnly,Category = "MovingControl")
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditDefaultsOnly,Category = "Combat")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditDefaultsOnly,Category = "Combat")
	TObjectPtr<UInputAction> ReloadAction;

	UPROPERTY(EditDefaultsOnly,Category = "Combat")
	TObjectPtr<UInputAction> GrenadeAction;
	
	// Poll for any relelvant classes and initialize our HUD
	void PollInit();
	
private:
	UPROPERTY(VisibleAnywhere,Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere,Category = Camera)
	class UCameraComponent* FollowCamera;

	/** 
	* Blaster components
	*/
	UPROPERTY(VisibleAnywhere,BlueprintReadOnly,meta = (AllowPrivateAccess = "true"))
	UCombatComponent* CombatComponent;

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* Buff;

	UPROPERTY(VisibleAnywhere)
	class ULagCompensationComponent* LagCompensation;


	
	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed();
	
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWS_Weapon* OverlappingWeapon;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;
	
	UFUNCTION()
	void OnRep_OverlappingWeapon(AWS_Weapon* LastWeapon);

	/** 
	* Animation montages
	*/
	
	UPROPERTY(EditAnywhere, Category = Combat)
	class UAnimMontage* FireWeaponMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ReloadMontage;
	
	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ElimMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* ThrowGrenadeMontage;

	UPROPERTY(EditAnywhere, Category = Combat)
	UAnimMontage* SwapMontage;
	
	bool bIsCrouched {false};

	bool bIsAiming {false};

	float AO_Yaw;

	float InterpAO_Yaw;
	
	float AO_Pitch;

	FRotator StartingAimRotation;

	ETurningInPlace TurningInPlace;

	void HideCameraIfCharacterClose();

	UPROPERTY(EditAnywhere)
	float CameraThreshold = 200.f;
	
	bool bRotateRootBone;
	
	float TurnThreshold = 0.5f;
	
	FRotator ProxyRotationLastFrame;
	
	FRotator ProxyRotation;
	
	float ProxyYaw;
	
	float TimeSinceLastMovementReplication;
	
	float CalculateSpeed();

	/**
	* Player health
	*/

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "Player Stats")
	float Health = 100.f;

	UFUNCTION()
	void OnRep_Health(float LastHealth);

	/** 
	* Player shield
	*/

	UPROPERTY(EditAnywhere, Category = "Player Stats")
	float MaxShield = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_Shield, VisibleAnywhere, Category = "Player Stats")
	float Shield = 0.f;

	UFUNCTION()
	void OnRep_Shield(float LastShield);
	
	UPROPERTY()
	class AWS_PlayerControll* PlayerController;

	bool bElimmed = false;

	FTimerHandle ElimTimer;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay;

	void ElimTimerFinished();

	bool bLeftGame = false;
	
	/**
	* Dissolve effect
	*/

	UPROPERTY(VisibleAnywhere)
	UTimelineComponent* DissolveTimeline;
	FOnTimelineFloat DissolveTrack;

	UPROPERTY(EditAnywhere)
	UCurveFloat* DissolveCurve;

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);
	void StartDissolve();

	// Dynamic instance that we can change at runtime
	UPROPERTY(VisibleAnywhere, Category = Elim)
	UMaterialInstanceDynamic* DynamicDissolveMaterialInstance;

	// Material instance set on the Blueprint, used with the dynamic material instance
	UPROPERTY(EditAnywhere, Category = Elim)
	UMaterialInstance* DissolveMaterialInstance;

	/**
	* Elim effects
	*/

	UPROPERTY(EditAnywhere)
	UParticleSystem* ElimBotEffect;

	UPROPERTY(VisibleAnywhere)
	UParticleSystemComponent* ElimBotComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue* ElimBotSound;

	UPROPERTY()
	class ABlasterPlayerState* BlasterPlayerState;

	
	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* CrownSystem;

	UPROPERTY()
	class UNiagaraComponent* CrownComponent;


	/** 
	* Grenade
	*/

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* AttachedGrenade;

	/** 
	* Default weapon
	*/

	UPROPERTY(EditAnywhere)
	TSubclassOf<AWS_Weapon> DefaultWeaponClass;


	
};
