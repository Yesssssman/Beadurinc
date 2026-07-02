#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ProfileSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FGameProfile
{
	GENERATED_BODY()
	
	/** A campaign save name named by a user */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save Data")
	FString CampaignName;
	
	/** A save game slot identifier name defined by program */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save Data")
	FString SaveSlotName;
	
	/**  */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save Data")
	FString LastWorld;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save Data")
	FDateTime LastPlayed;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save Data")
	int64 TotalPlayTime;
};

/**
 * Compact game save metadata that holds each saved campaigns in an array
 */
UCLASS()
class BEADURINC_API UProfileSaveGame : public USaveGame
{
	GENERATED_BODY()
	
public:
	
	/** @return slot name of save game */
	FString& PushNewGameProfile(const FString& WorldName);
	
	void DeleteSavedProfile(const FString& SaveSlotName);
	
private:
	
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, meta = (AllowPrivateAccess = true))
	TArray<FGameProfile> Profiles;
	
	/** Counter for the number of the games that user have created. Used for determining savegame slot name */
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, meta = (AllowPrivateAccess = true))
	int Seq;
};
