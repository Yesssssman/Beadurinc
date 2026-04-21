// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Task/BTTask_ProgressByGameplayTags.h"
#include "AIController.h"
#include "Actor/Character/FighterCharacter.h"

/** Constructor */
UBTTask_ProgressByGameplayTags::UBTTask_ProgressByGameplayTags()
{
	NodeName = "Check GameplayTag";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_ProgressByGameplayTags::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	return ValidateOwnerState(OwnerComp);
}

void UBTTask_ProgressByGameplayTags::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	EBTNodeResult::Type NodeState = ValidateOwnerState(OwnerComp);
	
	if (NodeState != EBTNodeResult::InProgress)
	{
		FinishLatentTask(OwnerComp, NodeState);
	}
}

/** Validate and return node result type based on GameplayTags */
EBTNodeResult::Type UBTTask_ProgressByGameplayTags::ValidateOwnerState(UBehaviorTreeComponent& OwnerComp)
{
	AAIController* const AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;
	
	AFighterCharacter* const AICharacter = Cast<AFighterCharacter>(AIController->GetPawn());
	if (!AICharacter) return EBTNodeResult::Failed;
	
	if (FAnimMontageInstance* ActiveMontageInstance = AICharacter->GetMesh()->GetAnimInstance()->GetActiveMontageInstance())
	{
		// When elapsed time less than blend time: wait until notify state set
		if (ActiveMontageInstance->GetPosition() < ActiveMontageInstance->GetBlendTime())
		{
			return EBTNodeResult::InProgress;
		}
	}
	
	UAbilitySystemComponent* ASC = AICharacter->GetAbilitySystemComponent();
	if (!ASC) return EBTNodeResult::Failed;
	
	// Found any failure tags: failed right away
	if (ASC->HasAnyMatchingGameplayTags(FailureTags))
	{
		return EBTNodeResult::Failed;
	}
	
	if (ASC->HasAnyMatchingGameplayTags(InProgressTags))
	{
		return EBTNodeResult::InProgress;
	}
	
	return EBTNodeResult::Succeeded;
}