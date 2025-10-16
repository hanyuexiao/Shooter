// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterGamemode.generated.h"

namespace MatchState
{
	extern MENUSYSTEM_API const FName Cooldown; // Match duration has been reached. Display winner and begin cooldown timer.
}

/**
 * 
 */
UCLASS()
class MENUSYSTEM_API ABlasterGamemode : public AGameMode
{
	GENERATED_BODY()

public:
	
	ABlasterGamemode();
	
	virtual void Tick(float DeltaTime) override;
	
	virtual void PlayerEliminated(class AWS_Player* ElimmedCharacter, class AWS_PlayerControll* VictimController, AWS_PlayerControll* AttackerController);

	virtual void RequestRespawn(ACharacter* ElimmedCharacter,AController* ElimmedController);

	void PlayerLeftGame(class ABlasterPlayerState* PlayerLeaving);
	
	UPROPERTY(EditDefaultsOnly)
	float WarmupTime = 10.f;
	
	UPROPERTY(EditDefaultsOnly)
	float MatchTime = 120.f;

	UPROPERTY(EditDefaultsOnly)
	float CooldownTime = 10.f;
	
	float LevelStartingTime = 0.f;

protected:
	virtual void BeginPlay() override;

	virtual void OnMatchStateSet() override;
private:
	float CountdownTime = 0.f;
};
