#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "StateWindowAnimNotifyState.generated.h"

class AFighterCharacter;
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
	
public:
	
	virtual void NotifyBegin(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent * MeshComp, UAnimSequenceBase * Animation, const FAnimNotifyEventReference& EventReference) override;
	
	virtual void BranchingPointNotifyBegin(FBranchingPointNotifyPayload& BranchingPointPayload) override;
	virtual void BranchingPointNotifyEnd(FBranchingPointNotifyPayload& BranchingPointPayload) override;
	
private:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "StateTag", meta = (AllowPrivateAccess = true))
	FGameplayTagContainer StateTags;
};