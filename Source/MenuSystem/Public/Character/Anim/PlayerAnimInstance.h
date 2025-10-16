// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "Character/BlasterTypes/TurningInPlace.h"
#include "PlayerAnimInstance.generated.h"

class UCombatComponent;
/**
 * 
 */
UCLASS()
class MENUSYSTEM_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
private:
	UPROPERTY(BlueprintReadWrite,Category = "Character",meta = (AllowPrivateAccess = "true"))
	class AWS_Player* PlayerCharacter;
	
	UPROPERTY(BlueprintReadWrite,Category = "Character",meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(BlueprintReadWrite,Category = "Character",meta = (AllowPrivateAccess = "true"))
	bool bIsInAir{false};
	
	UPROPERTY(BlueprintReadWrite,Category = "Character",meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating{false};

	UPROPERTY(BlueprintReadOnly,Category = "Character",meta = (AllowPrivateAccess = "true"))
	bool bWeaponEquipped{false};

	UPROPERTY(BlueprintReadOnly,Category = "Character",meta = (AllowPrivateAccess = "true"))
	bool bIsCrouched{false};

	UPROPERTY(BlueprintReadOnly,Category = "Character",meta = (AllowPrivateAccess = "true"))
	bool bIsAiming{false};

	UPROPERTY(BlueprintReadOnly,Category = "Character",meta = (AllowPrivateAccess = "true"))
	float YawOffset;

	UPROPERTY(BlueprintReadOnly,Category = "Character",meta = (AllowPrivateAccess = "true"))
	float Lean;
	
	UPROPERTY(BlueprintReadOnly, Category ="Character", meta = (AllowPrivateAccess = "true"))
	FRotator DeltaRotation;

	UPROPERTY(BlueprintReadOnly,Category = "Character",meta = (AllowPrivateAccess = "true"))
	float AO_Yaw;
	
	UPROPERTY(BlueprintReadOnly,Category = "Character",meta = (AllowPrivateAccess = "true"))
	float AO_Pitch;

	UPROPERTY(BlueprintReadOnly,Category = "Character",meta = (AllowPrivateAccess = "true"))
	FTransform LeftHandTransform;

	UPROPERTY(BlueprintReadOnly,Category = "Character",meta = (AllowPrivateAccess = "true"))
	ETurningInPlace TurningInPlace;
	
	FRotator CharacterRotation;
	FRotator CharacterRotationLastFrame;
	class AWS_Weapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FRotator RightHandRotation;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bLocallyControlled;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bRotateRootBone;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bElimmed;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bUseFABRIK;
	
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bUseAimOffsets;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bTransformRightHand;
};
