#include "AI/Decorator/BTDecorator_CheckEnemyState.h"

#include "Actor/Character/FighterCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_CheckEnemyState::UBTDecorator_CheckEnemyState()
{
	// Sets a node name in the Behavior Tree
	NodeName = "State Check for Enemy";
}

bool UBTDecorator_CheckEnemyState::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	const AFighterCharacter* Target = CastChecked<AFighterCharacter>(BlackboardComponent->GetValueAsObject(TargetActorKey.SelectedKeyName));
	
	// In case target is null (ideally, this case should be filtered by upper nodes. This is just a crash guard)
	if (!Target) return false;
	
	return Target->GetAbilitySystemComponent()->HasAllMatchingGameplayTags(StateTags);
}
