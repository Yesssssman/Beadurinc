#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTDecorator_Score.generated.h"

UCLASS()
class BEADURINC_API UBTDecorator_Score : public UBTDecorator
{
	GENERATED_BODY()
	
public:
	
	/** Constructor */
	UBTDecorator_Score();
	
protected:
	
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	
private:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard", meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector ScoreKeySelector;
	
};
