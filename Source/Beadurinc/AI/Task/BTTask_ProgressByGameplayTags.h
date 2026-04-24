#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_ProgressByGameplayTags.generated.h"

/**
 * A node that holds the BT flow based on GameplayTags
 */
UCLASS()
class BEADURINC_API UBTTask_ProgressByGameplayTags : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	
	/** Constructor */
	UBTTask_ProgressByGameplayTags();
	
protected:
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	/** Tick this task */
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
private:
	
	/** Validate and return node result type based on GameplayTags */
	EBTNodeResult::Type ValidateOwnerState(UBehaviorTreeComponent& OwnerComp);
	
private:
	
	/** Tag that represents the task is in-progress */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Tags", meta = (allowPrivateAccess = true))
	FGameplayTagContainer InProgressTags;
	
	/** Tag that represents the task failed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Tags", meta = (allowPrivateAccess = true))
	FGameplayTagContainer FailureTags;
};
