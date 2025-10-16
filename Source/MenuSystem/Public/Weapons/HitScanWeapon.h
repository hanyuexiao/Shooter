// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WS_Weapon.h"
#include "HitScanWeapon.generated.h"

UCLASS()
class MENUSYSTEM_API AHitScanWeapon : public AWS_Weapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;

	
protected:
	

	void WeaponTraceHit(const FVector& TraceStart, const FVector& HitTarget, FHitResult& OutHit);
	
	UPROPERTY(EditAnywhere)
	class UParticleSystem* ImpactParticles;

	UPROPERTY(EditAnywhere)
	USoundCue* HitSound;

private:
	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	USoundCue* FireSound;
	
};
