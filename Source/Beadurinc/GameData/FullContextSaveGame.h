#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "FullContextSaveGame.generated.h"

/**
 * `SaveGame` that holds full context of the game progression
 */
UCLASS()
class BEADURINC_API UFullContextSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	
	UFullContextSaveGame();
	
public:
	
	UPROPERTY(VisibleAnywhere, Category = Basic)
	FString RecentWorld;
	
	UPROPERTY(VisibleAnywhere, Category = Basic)
	FString SaveSlotName;
};
