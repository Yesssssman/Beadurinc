#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_WaitTag.generated.h"

UCLASS()
class BEADURINC_API UBTTask_WaitTag : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	
	/** Constructor */
	UBTTask_WaitTag();
	
public:
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
protected:
	
	/** Ticking task */
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
public:
	
	/** Tag that represents the task is in-progress */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Tags", meta = (allowPrivateAccess = true))
	FGameplayTagContainer InProgressTags;
	
	/** Tag that represents the task failed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Tags", meta = (allowPrivateAccess = true))
	FGameplayTagContainer FailureTags;
};
