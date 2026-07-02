#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelData.generated.h"

UCLASS()
class BEADURINC_API ULevelData : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UWorld> Level;
	
	UPROPERTY(EditAnywhere)
	FName DisplayNameKey;
	
	UPROPERTY(EditAnywhere)
	UTexture2D* Thumbnail;
};