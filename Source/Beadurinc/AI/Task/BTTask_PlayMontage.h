// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/ValueOrBBKey.h"
#include "BTTask_PlayMontage.generated.h"

/**
 *
 */
UCLASS()
class BEADURINC_API UBTTask_PlayMontage : public UBTTaskNode
{
	GENERATED_BODY()

public:

	UBTTask_PlayMontage();

protected:

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** Tick this task */
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	
	/** Latent task for montage finishes */
	void OnMontageFinished(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp) const;

private:
	/** Animation Montage to play. Must compatible with the skeleton that BT owner has */
	UPROPERTY(Category = Node, EditAnywhere)
	FValueOrBBKey_Object MontageToPlay = TObjectPtr<UAnimMontage>();
	
	/** State gameplay tags to finish the task */
	UPROPERTY(Category = Node, EditAnywhere)
	FGameplayTagContainer FinishConditionStates;
};