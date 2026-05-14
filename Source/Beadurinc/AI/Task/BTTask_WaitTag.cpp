#include "AI/Task/BTTask_WaitTag.h"

#include "AIController.h"
#include "Actor/Character/FighterCharacter.h"

UBTTask_WaitTag::UBTTask_WaitTag()
{
	NodeName = "Wait Tag";
	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_WaitTag::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// Returns InProgress at first
	return EBTNodeResult::Succeeded;
}

void UBTTask_WaitTag::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* const AIController = OwnerComp.GetAIOwner();
	if (!AIController) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }
	
	AFighterCharacter* const AICharacter = Cast<AFighterCharacter>(AIController->GetPawn());
	if (!AICharacter) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }
	
	UAbilitySystemComponent* ASC = AICharacter->GetAbilitySystemComponent();
	if (!ASC) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }
	
	// Found any failure tags: failed right away
	if (ASC->HasAnyMatchingGameplayTags(FailureTags))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}
	
	if (ASC->HasAnyMatchingGameplayTags(InProgressTags))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::InProgress);
		return;
	}
	
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}