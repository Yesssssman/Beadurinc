#include "AI/Service/BTService_BlockStaminaRegen.h"
#include "AIController.h"
#include "Actor/Character/FighterCharacter.h"

UBTService_BlockStaminaRegen::UBTService_BlockStaminaRegen()
{
	NodeName = "Block Stamina Regen";
	
	// Should activate notifies to run `OnBecomeRelevant`
	// (Deactivated as default for optimization)
	bNotifyBecomeRelevant = true;
}

void UBTService_BlockStaminaRegen::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* const AIController = OwnerComp.GetAIOwner();
	AFighterCharacter* OwnerCharacter = Cast<AFighterCharacter>(AIController->GetPawn());
	
	if (!OwnerCharacter) return;
	
	OwnerCharacter->ApplyStaminaRegenCooldown();
}