#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Confront.generated.h"

/**
 * Per-task instance memory. Tracks the spec handle activated by the event so
 * TickTask can poll for completion when bWaitForAbilityEnd is true.
 */
struct FBTConfrontMemory
{
	float RemainingTime;
	FVector MoveDirection;
};

UCLASS()
class BEADURINC_API UBTTask_Confront : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	
	/** Constructor */
	UBTTask_Confront();
	
public:
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	FORCEINLINE virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTConfrontMemory); }
	
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;

protected:
	
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
private:
	
	/** Base time how long the task node persists */
	UPROPERTY(EditAnywhere, Category = "Setup", meta = (allowPrivateAccess = true))
	float TimeWaiting;
	
	/** Extra time added to `TimeWaiting` on node activate */
	UPROPERTY(EditAnywhere, Category = "Setup", meta = (allowPrivateAccess = true))
	float TimeWaitingDistribution;
	
	UPROPERTY(EditAnywhere, Category = "Setup", meta = (allowPrivateAccess = true))
	float DistanceThreshold;
};
