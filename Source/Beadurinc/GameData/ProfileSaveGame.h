#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ProfileSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FGameProfile
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Save Data")
	FString SaveName;
	
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
	
private:
	
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, meta = (AllowPrivateAccess = true))
	TArray<FGameProfile> Profiles;
	
};
