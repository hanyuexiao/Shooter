// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/BlasterGamemode.h"

#include "Character/BlasterPlayerState.h"
#include "Character/WS_Player.h"
#include "Character/Controller/WS_PlayerControll.h"
#include "GameFramework/PlayerStart.h"
#include "GameState/BlasterGameState.h"
#include "Kismet/GameplayStatics.h"


namespace MatchState
{
	const FName Cooldown = FName("Cooldown");
}

ABlasterGamemode::ABlasterGamemode()
{
	bDelayedStart = true;
}

void ABlasterGamemode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartingTime = GetWorld() -> GetTimeSeconds();
}

void ABlasterGamemode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime < 0.f)
		{
			StartMatch();
		}
	}
	else if (MatchState == MatchState::InProgress)
	{
		CountdownTime = WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			SetMatchState(MatchState::Cooldown);
		}
	}
	else if (MatchState == MatchState::Cooldown)
	{
		CountdownTime = CooldownTime + WarmupTime + MatchTime - GetWorld()->GetTimeSeconds() + LevelStartingTime;
		if (CountdownTime <= 0.f)
		{
			RestartGame();
		}
	}
}

void ABlasterGamemode::OnMatchStateSet()
{
	Super::OnMatchStateSet();

	for (FConstPlayerControllerIterator It = GetWorld() -> GetPlayerControllerIterator(); It; ++It)
	{
		AWS_PlayerControll* BlasterPlayer = Cast<AWS_PlayerControll>(*It);
		if (BlasterPlayer)
		{
			BlasterPlayer -> OnMatchStateSet(MatchState);
		}
	}
}

void ABlasterGamemode::PlayerEliminated(class AWS_Player* ElimmedCharacter, class AWS_PlayerControll* VictimController,AWS_PlayerControll* AttackerController)
{
	if (AttackerController == nullptr || AttackerController->PlayerState == nullptr) return;
	if (VictimController == nullptr || VictimController->PlayerState == nullptr) return;
	ABlasterPlayerState* AttackerPlayerState = AttackerController ? Cast<ABlasterPlayerState>(AttackerController->PlayerState) : nullptr;
	ABlasterPlayerState* VictimPlayerState = VictimController ? Cast<ABlasterPlayerState>(VictimController->PlayerState) : nullptr;

	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	
	if (AttackerPlayerState && AttackerPlayerState != VictimPlayerState && BlasterGameState)
	{
		TArray<ABlasterPlayerState*> PlayersCurrentlyInTheLead;
		for (auto LeadPlayer : BlasterGameState->TopScoringPlayers)
		{
			PlayersCurrentlyInTheLead.Add(LeadPlayer);
		}

		AttackerPlayerState->AddToScore(1.f);
		BlasterGameState->UpdateTopScore(AttackerPlayerState);

		if (BlasterGameState->TopScoringPlayers.Contains(AttackerPlayerState))
		{
			AWS_Player* Leader = Cast<AWS_Player>(AttackerPlayerState->GetPawn());
			if (Leader)
			{
				Leader->MulticastGainedTheLead();
			}
		}

		for (int32 i = 0; i < PlayersCurrentlyInTheLead.Num(); i++)
		{
			if (!BlasterGameState->TopScoringPlayers.Contains(PlayersCurrentlyInTheLead[i]))
			{
				AWS_Player* Loser = Cast<AWS_Player>(PlayersCurrentlyInTheLead[i]->GetPawn());
				if (Loser)
				{
					Loser->MulticastLostTheLead();
				}
			}
		}
	}
	if (VictimPlayerState)
	{
		VictimPlayerState->AddToDefeats(1);
	}
	
	if (ElimmedCharacter)
	{
		ElimmedCharacter -> Elim(false);
	}
	
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		AWS_PlayerControll* BlasterPlayer = Cast<AWS_PlayerControll>(*It);
		if (BlasterPlayer && AttackerPlayerState && VictimPlayerState)
		{
			BlasterPlayer->BroadcastElim(AttackerPlayerState, VictimPlayerState);
		}
	}
}

void ABlasterGamemode::RequestRespawn(ACharacter* ElimmedCharacter, AController* ElimmedController)
{
	if (ElimmedCharacter)
	{
		ElimmedCharacter->Reset();
		ElimmedCharacter->Destroy();
	}
	if (ElimmedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 Selection = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(ElimmedController, PlayerStarts[Selection]);
	}
}

void ABlasterGamemode::PlayerLeftGame(class ABlasterPlayerState* PlayerLeaving)
{
	if (PlayerLeaving == nullptr) return;
	ABlasterGameState* BlasterGameState = GetGameState<ABlasterGameState>();
	if (BlasterGameState && BlasterGameState->TopScoringPlayers.Contains(PlayerLeaving))
	{
		BlasterGameState->TopScoringPlayers.Remove(PlayerLeaving);
	}
	AWS_Player* CharacterLeaving = Cast<AWS_Player>(PlayerLeaving->GetPawn());
	if (CharacterLeaving)
	{
		CharacterLeaving->Elim(true);
	}
}


