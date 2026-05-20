#include "MonsterCharacter.h"

#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Net/UnrealNetwork.h"

AMonsterCharacter::AMonsterCharacter()
{
	bReplicates = true;
}

void AMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent && HasAuthority())
	{
		if (IsValid(AIBlockAbility))
		{
			FGameplayAbilitySpec Spec(AIBlockAbility, 1, INDEX_NONE, this);
			AbilitySystemComponent->GiveAbility(Spec);
		}

		if (IsValid(AIParryAbility))
		{
			FGameplayAbilitySpec Spec(AIParryAbility, 1, INDEX_NONE, this);
			AbilitySystemComponent->GiveAbility(Spec);
		}
	}
}

void AMonsterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME( AMonsterCharacter, BTState );
}

void AMonsterCharacter::ModifyBlackboardScore(const FName& BlackboardKeyName, const int& BaseScore, const int& Distribution)
{
	AAIController* AsAIController = Cast<AAIController>(GetController());
	if (!AsAIController) return;
	
	UBlackboardComponent* BlackboardComponent = AsAIController->GetBlackboardComponent();
	if (!BlackboardComponent) return;
	
	// Increase Drawback score in BB component so that the character can retreat and take a breath
	int CurrentScore = BlackboardComponent->GetValueAsInt(BlackboardKeyName);
	int NextScore = FMath::Clamp(
		CurrentScore + BaseScore
			+ FMath::RandRange(Distribution < 0 ? Distribution : 0, Distribution < 0 ? 0 : Distribution),
		0, 100
	);
	
	BlackboardComponent->SetValueAsInt(BlackboardKeyName, NextScore);
}

