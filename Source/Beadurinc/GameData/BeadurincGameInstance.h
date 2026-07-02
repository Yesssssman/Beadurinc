#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BeadurincGameInstance.generated.h"

class UProfileSaveGame;
class UFullContextSaveGame;

UCLASS()
class BEADURINC_API UBeadurincGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
public:
	
	UBeadurincGameInstance();
	
public:
	
	UFUNCTION(BlueprintCallable, Category = "Game Save")
	void CreateNewGame(const FString& SlotName, const FString& WorldName);
	
	UFUNCTION(BlueprintCallable, Category = "Game Save")
	void DeleteSavedGame(const FString& SlotName);
	
	UFUNCTION(BlueprintCallable, Category = "Game Save")
	void LoadGame(const FString& SlotName);
	
	UFUNCTION(BlueprintCallable, Category = "Game Save")
	void SaveProgression();
	
private:
	
	/** Array of saved game profiles(metadata) */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadWrite, Category = "Game Save", meta = (AllowPrivateAccess = true))
	TObjectPtr<UProfileSaveGame> GameProfiles;
	
	/** Activated save game slot name */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Game Save", meta = (AllowPrivateAccess = true))
	FString ActivatedSaveGameSlotName;
	
	/** Activated save game instance */
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Game Save", meta = (AllowPrivateAccess = true))
	TObjectPtr<UFullContextSaveGame> ActivatedSaveGameInstance;
};
