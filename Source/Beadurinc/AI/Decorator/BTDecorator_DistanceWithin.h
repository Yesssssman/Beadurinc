#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_DistanceWithin.generated.h"

UCLASS()
class BEADURINC_API UBTDecorator_DistanceWithin : public UBTDecorator
{
	GENERATED_BODY()

public:

	/** Constructor */
	UBTDecorator_DistanceWithin();

protected:

	/** Returns a check result of decorator */
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

private:
	/** The Blackboard key for target actor */
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector TargetActorKey;

	/** Min distance boundary */
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (AllowPrivateAccess = true))
	float Min;
	
	/** Max distance boundary */
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (AllowPrivateAccess = true))
	float Max;
};