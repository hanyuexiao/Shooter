// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BlasterPlayerState.h"

#include "Character/WS_Player.h"
#include "Character/Controller/WS_PlayerControll.h"
#include "Net/UnrealNetwork.h"


void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerState, Defeats);
}

void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	Character = Character == nullptr ? Cast<AWS_Player>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AWS_PlayerControll>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == nullptr ? Cast<AWS_Player>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AWS_PlayerControll>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDScore(GetScore());
		}
	}
}

void ABlasterPlayerState::AddToDefeats(int32 DefeatsAmount)
{
	Defeats += DefeatsAmount;
	Character = Character == nullptr ? Cast<AWS_Player>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AWS_PlayerControll>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}

void ABlasterPlayerState::OnRep_Defeats()
{
	Character = Character == nullptr ? Cast<AWS_Player>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == nullptr ? Cast<AWS_PlayerControll>(Character->Controller) : Controller;
		if (Controller)
		{
			Controller->SetHUDDefeats(Defeats);
		}
	}
}