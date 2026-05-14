#include "AI/Decorator/BTDecorator_Score.h"

#include "BehaviorTree/BlackboardComponent.h"

UBTDecorator_Score::UBTDecorator_Score()
{
	NodeName = "Score";
}

bool UBTDecorator_Score::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const UBlackboardComponent* BBComp = OwnerComp.GetBlackboardComponent();
	if (!BBComp) return false; 
	
	const int32 CurrentScore = BBComp->GetValueAsInt(ScoreKeySelector.SelectedKeyName);

	// Score calculation: current score represents the precent of true
	return FMath::FRand() <= CurrentScore / 100.0F;
}