// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Anim/PlayerAnimInstance.h"
#include "Character/WS_Player.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "Weapons/WS_Weapon.h"


void UPlayerAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	
	PlayerCharacter = Cast<AWS_Player>(TryGetPawnOwner());
	

}

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	
	if (PlayerCharacter == nullptr) PlayerCharacter = Cast<AWS_Player>(TryGetPawnOwner());
	if (PlayerCharacter == nullptr) return;

	FVector Velocity = PlayerCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = PlayerCharacter -> GetCharacterMovement() -> IsFalling();
	bIsAccelerating = PlayerCharacter -> GetCharacterMovement() -> GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = PlayerCharacter -> IsWeaponEquipped();
	EquippedWeapon = PlayerCharacter -> GetEquippedWeapon();
	bIsCrouched = PlayerCharacter -> IsCrouched();
	bIsAiming = PlayerCharacter -> IsAiming();
	TurningInPlace = PlayerCharacter -> GetTurningInPlace();
	bRotateRootBone = PlayerCharacter->ShouldRotateRootBone();
	bElimmed = PlayerCharacter -> IsElimmed();
	
	// 瞄准方向
	FRotator AimRotation = PlayerCharacter -> GetBaseAimRotation();

	// 速度方向
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(PlayerCharacter ->GetVelocity());
	
	const FRotator TargetDeltaRotation  = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation,TargetDeltaRotation,DeltaSeconds,6.f);
	YawOffset = DeltaRotation.Yaw;
	
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = PlayerCharacter -> GetActorRotation();
	const FRotator Delata = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotationLastFrame,CharacterRotation);
	const float Target = Delata.Yaw / DeltaSeconds;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaSeconds, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = PlayerCharacter->GetAoYaw();
	AO_Pitch = PlayerCharacter->GetAoPitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && PlayerCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		PlayerCharacter ->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"),ERelativeTransformSpace::RTS_World);
		FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));
	//	DrawDebugLine(GetWorld(),MuzzleTipTransform.GetLocation(),MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f,FColor::Red);
	 	DrawDebugLine(GetWorld(),MuzzleTipTransform.GetLocation(),PlayerCharacter -> GetHitTarget() ,FColor::Blue);
		
		if (PlayerCharacter -> IsLocallyControlled())
		{
			bLocallyControlled = true;
			//FTransform RightHandTransform =PlayerCharacter->GetMesh()->GetSocketTransform(FName("hand_r"), RTS_World);

			//RightHandRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - PlayerCharacter->GetHitTarget()));
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - PlayerCharacter -> GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaSeconds, 30.f);
		}
	}
	
	bUseFABRIK = PlayerCharacter -> GetCombatState() != ECombatState::ECS_Reloading;

	bool bFABRIKOverride = PlayerCharacter->IsLocallyControlled() &&
	PlayerCharacter->GetCombatState() != ECombatState::ECS_ThrowingGrenade &&
	PlayerCharacter->bFinishedSwapping;
	if (bFABRIKOverride)
	{
		bUseFABRIK = !PlayerCharacter->IsLocallyReloading();
	}

	bUseAimOffsets = PlayerCharacter -> GetCombatState() != ECombatState::ECS_Reloading;
	
	bTransformRightHand = PlayerCharacter -> GetCombatState() != ECombatState::ECS_Reloading;
}

