#include "AI/Decorator/BTDecorator_Random.h"

UBTDecorator_Random::UBTDecorator_Random()
{
	NodeName = "Random";
}

bool UBTDecorator_Random::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return FMath::FRand() <= Chance;
}
