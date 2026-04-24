#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimMetaData.h"
#include "AttackMetaData.generated.h"

UCLASS()
class BEADURINC_API UAttackMetaData : public UAnimMetaData
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag")
	FGameplayTag AttackTypeTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stun")
	TObjectPtr<UAnimMontage> OnParried; 
};
