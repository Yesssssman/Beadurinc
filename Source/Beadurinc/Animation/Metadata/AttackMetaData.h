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
	
	/** A tag for unblockable attack type of an attack animation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag")
	FGameplayTag DangerAttackTypeTag;
	
	/** A tag to indicate which parry animation to play, only allows left and right. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag")
	FGameplayTag ParryDirectionTag;
	
	/** A tag to determine stun animation */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag")
	FGameplayTag StunTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Stun")
	TObjectPtr<UAnimMontage> OnParried; 
};
