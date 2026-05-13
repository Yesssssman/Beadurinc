#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_Random.generated.h"

UCLASS()
class BEADURINC_API UBTDecorator_Random : public UBTDecorator
{
	GENERATED_BODY()
	
public:

	/** Constructor */
	UBTDecorator_Random();

protected:

	/** Returns a check result of decorator */
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;

private:
	
	/** Random % to pass the test */
	UPROPERTY(EditAnywhere, Category = "Condition", meta = (AllowPrivateAccess = true))
	float Chance;
};
