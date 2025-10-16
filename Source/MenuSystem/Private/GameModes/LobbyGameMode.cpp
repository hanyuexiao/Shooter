// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (GameState)
	{
		// 获取有多少玩家
		int32 NumberOfPlayers = GameState.Get() -> PlayerArray.Num();

		if (GEngine)
		{
			GEngine -> AddOnScreenDebugMessage(
					1,
					600.f,
					FColor::Yellow,
					FString::Printf(TEXT("Playersd in game: %d"),NumberOfPlayers)
					);

			APlayerState* PlayerState = NewPlayer->GetPlayerState<APlayerState>();
			if (PlayerState)
				{
					FString PlayerName = PlayerState->GetPlayerName();
					GEngine->AddOnScreenDebugMessage(
						2,
						60.f,
						FColor::Cyan,
						FString::Printf(TEXT("%s has joined the game!"), *PlayerName)
					);
				}
			if (NumberOfPlayers == 2)
				{
					UWorld* World = GetWorld();
					if (World)
					{
						World -> ServerTravel(FString("/Game/Maps/StartMap?listen"));
					}
				}
		}
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	APlayerState* PlayerState = Exiting->GetPlayerState<APlayerState>();
	
	if (PlayerState)
	{
		// 获取剩余多少个玩家数量
		int32 NumberOfPlayers = GameState.Get() -> PlayerArray.Num();
		
		GEngine->AddOnScreenDebugMessage(
		1,
		600.f,
		FColor::Yellow,
		FString::Printf(TEXT("Players in game: %d"), NumberOfPlayers - 1)
		);

		// 获取退出游戏的玩家名字
		FString PlayerName = PlayerState->GetPlayerName();
		
		GEngine->AddOnScreenDebugMessage(
			2,
			60.f,
			FColor::Cyan,
			FString::Printf(TEXT("%s has exited the game!"), *PlayerName)
		);
	}
}
