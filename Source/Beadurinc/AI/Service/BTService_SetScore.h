#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_SetScore.generated.h"

UCLASS()
class BEADURINC_API UBTService_SetScore : public UBTService
{
	GENERATED_BODY()
	
public:
	
	/** Constructor */
	UBTService_SetScore();
	
protected:
	
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
private:
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blackboard", meta = (AllowPrivateAccess = true))
	FBlackboardKeySelector ScoreKeySelector;
	
	/** A base modifier value that is fixed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setter", meta = (AllowPrivateAccess = true))
	int Amount;
	
	/** A minimum modifier distribution value that is varying within DistributorMin - DistributorMax*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setter", meta = (AllowPrivateAccess = true))
	int DistributorMin;
	
	/** A maximum modifier distribution value that is varying within DistributorMin - DistributorMax*/
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setter", meta = (AllowPrivateAccess = true))
	int DistributorMax;
};
