#include "AI/Service/BTService_SetScore.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "EnvironmentQuery/EnvQueryTypes.h"

UBTService_SetScore::UBTService_SetScore()
{
	// Should activate notifies to run `OnBecomeRelevant` and `OnCeaseRelevant`
	// (Deactivated as default for optimization)
	bNotifyBecomeRelevant = true;
	bNotifyCeaseRelevant = false;
	NodeName = "Set Score";
}

void UBTService_SetScore::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	Super::OnBecomeRelevant(OwnerComp, NodeMemory);
	
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComponent) return;
	
	int CurrentScore = BlackboardComponent->GetValueAsInt(ScoreKeySelector.SelectedKeyName);
	int NextScore = FMath::Clamp(CurrentScore + Amount + FMath::RandRange(DistributorMin, DistributorMax), 0, 100);
	
	BlackboardComponent->SetValueAsInt(ScoreKeySelector.SelectedKeyName, NextScore);
}
