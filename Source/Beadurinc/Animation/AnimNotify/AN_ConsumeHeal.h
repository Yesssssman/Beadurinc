#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "AN_ConsumeHeal.generated.h"

/**
 * Consumes healing potion declared in `ABeadurincPlayerState`
 */
UCLASS()
class BEADURINC_API UAN_ConsumeHeal : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
