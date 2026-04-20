#include "AI/Service/BTService_SetRepState.h"

#include "AIController.h"
#include "Actor/Character/MonsterCharacter.h"

UBTService_SetRepState::UBTService_SetRepState()
{
	NodeName = "Set Replicate State";
	
	// Should activate notifies to run `OnBecomeRelevant` and `OnCeaseRelevant`
	// (Deactivated as default for optimization)
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
}

void UBTService_SetRepState::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	
	AAIController* const AIController = OwnerComp.GetAIOwner();
	AMonsterCharacter* OwnerCharacter = Cast<AMonsterCharacter>(AIController->GetPawn());
	
	if (!OwnerCharacter) return;
	OwnerCharacter->SetBTState(StateOnRelevant);
}

void UBTService_SetRepState::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
	
	AAIController* const AIController = OwnerComp.GetAIOwner();
	AMonsterCharacter* OwnerCharacter = Cast<AMonsterCharacter>(AIController->GetPawn());
	
	if (!OwnerCharacter) return;
	OwnerCharacter->SetBTState(StateOnCease);
}
