// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"


class AWS_Player;

USTRUCT(BlueprintType)
struct FBoxInformation
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Location;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	FVector BoxExtent;
};

USTRUCT(BlueprintType)
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	float Time;

	UPROPERTY()
	TMap<FName, FBoxInformation> HitBoxInfo;

	UPROPERTY()
	AWS_Player* Character;
};

USTRUCT(BlueprintType)
struct FServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHitConfirmed;

	UPROPERTY()
	bool bHeadShot;
};

USTRUCT(BlueprintType)
struct FShotgunServerSideRewindResult
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<AWS_Player*, uint32> HeadShots;

	UPROPERTY()
	TMap<AWS_Player*, uint32> BodyShots;

};


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MENUSYSTEM_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	ULagCompensationComponent();
	friend class AWS_Player;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void ShowFramePackage(const FFramePackage& Package, const FColor& Color);

	
	/** 
	* Hitscan
	*/
	FServerSideRewindResult ServerSideRewind(
		class AWS_Player* HitCharacter, 
		const FVector_NetQuantize& TraceStart, 
		const FVector_NetQuantize& HitLocation, 
		float HitTime);
	
	/** 
	* Projectile
	*/
	FServerSideRewindResult ProjectileServerSideRewind(
		AWS_Player* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	/** 
	* Shotgun
	*/
	FShotgunServerSideRewindResult ShotgunServerSideRewind(
	const TArray<AWS_Player*>& HitCharacters,
	const FVector_NetQuantize& TraceStart,
	const TArray<FVector_NetQuantize>& HitLocations,
	float HitTime);

	
	UFUNCTION(Server, Reliable)
	void ServerScoreRequest(
		AWS_Player* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation,
		float HitTime
	);

	UFUNCTION(Server, Reliable)
	void ShotgunServerScoreRequest(
		const TArray<AWS_Player*>& HitCharacters,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations,
		float HitTime
	);
	
	UFUNCTION(Server, Reliable)
	void ProjectileServerScoreRequest(
		AWS_Player* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);
protected:
	
	virtual void BeginPlay() override;

	void SaveFramePackage(FFramePackage& Package);

	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
	
	void CacheBoxPositions(AWS_Player* HitCharacter, FFramePackage& OutFramePackage);
	void MoveBoxes(AWS_Player* HitCharacter, const FFramePackage& Package);
	void ResetHitBoxes(AWS_Player* HitCharacter, const FFramePackage& Package);
	void EnableCharacterMeshCollision(AWS_Player* HitCharacter, ECollisionEnabled::Type CollisionEnabled);
	void SaveFramePackage();
	FFramePackage GetFrameToCheck(AWS_Player* HitCharacter, float HitTime);

	
	/** 
	* Hitscan
	*/
	FServerSideRewindResult ConfirmHit(
		const FFramePackage& Package,
		AWS_Player* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& HitLocation);
	

	/** 
	* Projectile
	*/
	FServerSideRewindResult ProjectileConfirmHit(
		const FFramePackage& Package,
		AWS_Player* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize100& InitialVelocity,
		float HitTime
	);

	/** 
	* Shotgun
	*/

	FShotgunServerSideRewindResult ShotgunConfirmHit(
		const TArray<FFramePackage>& FramePackages,
		const FVector_NetQuantize& TraceStart,
		const TArray<FVector_NetQuantize>& HitLocations
	);
private:

	UPROPERTY()
	AWS_Player* Character;

	UPROPERTY()
	class AWS_PlayerControll* Controller;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere)
	float MaxRecordTime = 4.f;
public:	


};
