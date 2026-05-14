#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CheckEnemyState.generated.h"

/**
 * Examine whether gameplay tag is currently included in an opponent.
 */
UCLASS()
class BEADURINC_API UBTDecorator_CheckEnemyState : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	
	/** Constructor */
	UBTDecorator_CheckEnemyState();
	
protected:
	
	/** Returns a validation result of decorator */
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
private:
	
	/** The Blackboard key for target actor to check tags */
	UPROPERTY(EditAnywhere, Category = "Blackboard", meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector TargetActorKey;
	
	/**
	 * A gameplay tags to check from opponent's Ability System Component
	 * The operation is *ALL* matching, not *ANY*
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Ability", meta = (AllowPrivateAccess = true))
	FGameplayTagContainer StateTags;
	
};
