#include "AI/Service/BTService_ModifyMoveSpeed.h"

#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTService_ModifyMoveSpeed::UBTService_ModifyMoveSpeed()
{
	// Should activate notifies to run `OnBecomeRelevant` and `OnCeaseRelevant`
	// (Deactivated as default for optimization)
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = true;
	
	NodeName = "Modify Move Speed";
}

void UBTService_ModifyMoveSpeed::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	
	AAIController* const AIController = OwnerComp.GetAIOwner();
	ACharacter* OwnerCharacter = Cast<ACharacter>(AIController->GetPawn());
	
	if (!OwnerCharacter) return;
	
	OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed += SpeedAddition;
}

void UBTService_ModifyMoveSpeed::OnCeaseRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnCeaseRelevant(OwnerComp, NodeMemory);
	
	AAIController* const AIController = OwnerComp.GetAIOwner();
	ACharacter* OwnerCharacter = Cast<ACharacter>(AIController->GetPawn());
	
	if (!OwnerCharacter) return;
	
	OwnerCharacter->GetCharacterMovement()->MaxWalkSpeed -= SpeedAddition;
}
