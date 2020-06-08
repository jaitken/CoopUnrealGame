// Fill out your copyright notice in the Description page of Project Settings.


#include "SGameMode.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "SGameState.h"
#include "SPlayerState.h"
#include "Components/SHealthComponent.h"
#include "EngineUtils.h"




void ASGameMode::SetGameStarted(bool StartGame)
{
	GameStarted = StartGame;
}

ASGameMode::ASGameMode()
{
	TimeBetweenWaves = 5.0f;

	GameStateClass = ASGameState::StaticClass();
	PlayerStateClass = ASPlayerState::StaticClass();

	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickInterval = 1.0f;

}

void ASGameMode::PrepareForNextWave()
{

	RestartDeadPlayers();

	GetWorldTimerManager().SetTimer(TimerHandle_NextWaveStart, this, &ASGameMode::StartWave, TimeBetweenWaves, false);

	SetWaveState(EWaveState::WaitingToStart);

}


void ASGameMode::StartWave()
{
	WaveCount++;

	NumOfBotsToSpawn = 2 * WaveCount;

	GetWorldTimerManager().SetTimer(TimerHandle_BotSpawner, this, &ASGameMode::SpawnBotTimerElapsed, 1.0f, true, 0.0f);

	SetWaveState(EWaveState::WaveInProgress);


}


void ASGameMode::CheckWaveState()
{
	   
	
		bool bIsPreparingForWave = GetWorldTimerManager().IsTimerActive(TimerHandle_NextWaveStart);

		if (NumOfBotsToSpawn > 0 || bIsPreparingForWave)
		{
			return;
		}

		bool bIsAnyBotAlive = false;

		for (TActorIterator<AActor> ActorIterator(GetWorld()); ActorIterator; ++ActorIterator)
		{
			AActor* Actor = *ActorIterator;
			APawn* TestPawn = Cast<APawn>(Actor);

			if (TestPawn == nullptr || TestPawn->IsPlayerControlled())
			{
				continue;
			}

			USHealthComponent* HealthComp = Cast<USHealthComponent>(TestPawn->GetComponentByClass(USHealthComponent::StaticClass()));
			if (HealthComp && HealthComp->GetHealth() > 0.0f)
			{
				bIsAnyBotAlive = true;
				break;
			}
		}

		if (!bIsAnyBotAlive)
		{
			SetWaveState(EWaveState::WaveComplete);
			PrepareForNextWave();
		}
	
	

}


void ASGameMode::CheckAnyPlayerAlive()
{
		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
			{
				APlayerController* PC = It->Get();

				if (PC && PC->GetPawn())
				{
					APawn* MyPawn = PC->GetPawn();
					USHealthComponent* HealthComp = Cast<USHealthComponent>(MyPawn->GetComponentByClass(USHealthComponent::StaticClass()));

					if (ensure(HealthComp) && HealthComp->GetHealth() > 0.0f)
					{
						//player is still alive
						return;
					}
				}
			}
			//no player alive
			GameOver();
	
}


void ASGameMode::EndWave()
{

	GetWorldTimerManager().ClearTimer(TimerHandle_BotSpawner);
	
	SetWaveState(EWaveState::WaitingToComplete);
}





void ASGameMode::GameOver()
{
	EndWave();
	//TODO: finish match, present 'game over' to players
	SetWaveState(EWaveState::GameOver);


	UE_LOG(LogTemp, Log, TEXT("Game Over! Players are dead"))
}

void ASGameMode::SetWaveState(EWaveState NewState)
{
	ASGameState* GS = GetGameState<ASGameState>();
	if (ensureAlways(GS))
	{
		GS->SetWaveState(NewState);
	}
}

void ASGameMode::RestartDeadPlayers()
{

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();

		if (PC && PC->GetPawn() == nullptr)
		{
			RestartPlayer(PC);
		}
	}


}

void ASGameMode::StartPlay()
{
	UE_LOG(LogTemp, Log, TEXT(" ASGAMEMODE: StartPlay Called "))
	Super::StartPlay();

	//SetWaveState(EWaveState::MainMenu);
	//PrepareForNextWave();

}

void ASGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	ASGameState* GS = GetGameState<ASGameState>();
	if (GS->GetWaveState() == EWaveState::MainMenu)
	{
		return;
	}

	CheckWaveState();
	CheckAnyPlayerAlive();

}

void ASGameMode::SpawnBotTimerElapsed()
{

	SpawnNewBot();
	NumOfBotsToSpawn--;

	if (NumOfBotsToSpawn <= 0) 
	{
		EndWave();
	}

}
