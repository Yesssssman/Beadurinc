#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "BeadurincGameInstance.generated.h"

class UFullContextSaveGame;

UCLASS()
class BEADURINC_API UBeadurincGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
private:
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	FString SaveGameSlotName;
	
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = true))
	TObjectPtr<UFullContextSaveGame> SaveGameInstance;
	
};
