#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "StateWindowAnimNotifyState.generated.h"

/**
 * A notify state for determining a character's state by FGameplayTag.
 * 
 * At the beginning of notify, called on NotifyBegin, adds the pre-defined
 * gameplay tag to the owner's gameplay ability system. 
 */
UCLASS()
class BEADURINC_API UStateWindowAnimNotifyState : public UAnimNotifyState
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StateTag", meta = (AllowPrivateAccess = true))
	FGameplayTag StateTag;
	
protected:
	
	virtual void NotifyBegin
	(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float TotalDuration,
		const FAnimNotifyEventReference& EventReference
	) override;
	
	virtual void NotifyEnd
	(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;
};
