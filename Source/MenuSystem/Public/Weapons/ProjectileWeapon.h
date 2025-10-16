// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WS_Weapon.h"
#include "ProjectileWeapon.generated.h"

UCLASS()
class MENUSYSTEM_API AProjectileWeapon : public AWS_Weapon
{
	GENERATED_BODY()

public:
	AProjectileWeapon();
	
	virtual void Fire(const FVector& HitTarget) override;

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> ProjectileClass;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<AProjectile> ServerSideRewindProjectileClass;
	
	
};
