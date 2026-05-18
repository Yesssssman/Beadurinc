#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayAbilitySpecHandle.h"
#include "BTTask_ActivateAbility.generated.h"

/**
 * Per-task instance memory. Tracks the spec handle activated by the event so
 * TickTask can poll for completion when bWaitForAbilityEnd is true.
 */
struct FBTActivateAbilityMemory
{
	FGameplayAbilitySpecHandle TrackedHandle;
};

/**
 * Sends a gameplay event with AbilityTriggerTag to the AI pawn's ASC, which
 * activates a UGameplayAbility whose AbilityTriggers contain that tag.
 * When bWaitForAbilityEnd is true (default), the task stays InProgress until
 * the activated ability ends.
 */
UCLASS()
class BEADURINC_API UBTTask_ActivateAbility : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	
	/** Constructor */
	UBTTask_ActivateAbility();
	
public:
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	
	FORCEINLINE virtual uint16 GetInstanceMemorySize() const override { return sizeof(FBTActivateAbilityMemory); }
	
protected:
	
	/** Ticking task */
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
private:
	
	/** A gameplay tag that is bound to trigger an ability */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Ability", meta = (AllowPrivateAccess = true))
	FGameplayTag AbilityTriggerTag;
	
	/** Gameplay tags that should regard this task node as "InProgress" even tho the ability is deactivated */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Ability", meta = (AllowPrivateAccess = true))
	FGameplayTagContainer ProgressingTags;
	
	/** If true, the task stays InProgress until the activated ability ends. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Ability", meta = (AllowPrivateAccess = true))
	bool bWaitForAbilityEnd = true;
};