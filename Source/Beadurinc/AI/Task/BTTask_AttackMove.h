#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/ValueOrBBKey.h"
#include "BTTask_AttackMove.generated.h"

UCLASS()
class BEADURINC_API UBTTask_AttackMove : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
	
	/** Constructor */
	UBTTask_AttackMove();
	
private:
	
	/**
	 * Attack moves that shows alert icon ont player's top, the pierce and low attacks
	 * 
	 * Managed by container to ease checking
	 */
	static const FGameplayTagContainer DangerAttackTypes;
	
public:
	
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	
protected:
	
	/** Ticking task */
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	/** Multicast function to play a montage in locals too */
	UFUNCTION(NetMulticast, Reliable)
	void PlayMontageMulticast(ACharacter* CharacterIn, UAnimMontage* MontageIn);
	
private:
	
	/** Validate and return node result type based on GameplayTags */
	EBTNodeResult::Type ValidateOwnerState(UBehaviorTreeComponent& OwnerComp);
	
private:
	/** Animation Montage to play. Must compatible with the skeleton that BT owner has */
	UPROPERTY(Category = Animation, EditAnywhere)
	FValueOrBBKey_Object MontageToPlay = TObjectPtr<UAnimMontage>();
	
	/** Tag that represents the task is in-progress */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Tags", meta = (allowPrivateAccess = true))
	FGameplayTagContainer InProgressTags;
	
	/** Tag that represents the task failed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gameplay Tags", meta = (allowPrivateAccess = true))
	FGameplayTagContainer FailureTags;
	
	// The node only succeed when Ability System owner doesn't have tags neither `InProgressTags` nor `FailureTags`.
};
