#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_CheckState.generated.h"

UCLASS()
class BEADURINC_API UBTDecorator_CheckState : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	
    /** Constructor */
    UBTDecorator_CheckState();
	
protected:
	
	/** Returns a check result of decorator */
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
private:
	
	/** The tags to check whether included in owner actor's Ability System Component */
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (AllowPrivateAccess = true))
	FGameplayTagContainer States;
};
