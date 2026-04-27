#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_BlockStaminaRegen.generated.h"

/**
 * Apply stamina regeneration block effect if AI owner is AFighterCharacter
 */
UCLASS()
class BEADURINC_API UBTService_BlockStaminaRegen : public UBTService
{
	GENERATED_BODY()
	
public:
	
	/** Constructor */
	UBTService_BlockStaminaRegen();
	
protected:
	
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
