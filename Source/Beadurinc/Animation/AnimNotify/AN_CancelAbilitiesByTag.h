#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_CancelAbilitiesByTag.generated.h"

UCLASS()
class BEADURINC_API UAN_CancelAbilitiesByTag : public UAnimNotify
{
	GENERATED_BODY()
	
private:
	
	/** Gameplay tags to cancel abilities set by `Cancel Abilities with Tag` */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Tag", meta=(AllowPrivateAccess=true))
	FGameplayTagContainer TagsToCancelAbilities;
	
public:
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
};
