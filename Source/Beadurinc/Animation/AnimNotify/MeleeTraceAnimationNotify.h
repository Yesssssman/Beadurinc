#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "MeleeTraceAnimationNotify.generated.h"

UCLASS()
class BEADURINC_API UMeleeTraceAnimationNotify : public UAnimNotifyState
{
	GENERATED_BODY()
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
