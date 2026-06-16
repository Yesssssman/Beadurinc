#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ProfileSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FGameProfile
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FString LastWorld;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FDateTime LastPlayed;
};

UCLASS()
class BEADURINC_API UProfileSaveGame : public USaveGame
{
	GENERATED_BODY()
	
private:
	
	UPROPERTY(BlueprintReadOnly, VisibleDefaultsOnly, meta = (AllowPrivateAccess = true))
	TArray<FGameProfile> Profiles;
	
};
