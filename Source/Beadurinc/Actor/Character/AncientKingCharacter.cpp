#include "AncientKingCharacter.h"
#include "AbilitySystemComponent.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

// Sets default values
AAncientKingCharacter::AAncientKingCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	// Create GAS component
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
	// Replicated = true so client side ASC knows the updated values
	AbilitySystemComponent->SetIsReplicated(true);
	
	// Full -> The GAS components will be replicated all tracking clients
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Full);
}

void AAncientKingCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize only in authorized side to let them replicated to clients by networking
	if (AbilitySystemComponent && HasAuthority())
	{
		if (AttributeSetClass)
		{
			// Create AttributeSet
			// Note: AttributeSet data are initialized here unlike PlayerCharacter
			UAttributeSet* CreatedAttributeSet = NewObject<UAttributeSet>(this, AttributeSetClass);
			
			AttributeSet = CreatedAttributeSet;
			AbilitySystemComponent->AddAttributeSetSubobject(CreatedAttributeSet);
		}
		
		// Initialize Actor Info
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		
		if (InitialStatsTable)
		{
			// Initialize stats from data table 
			AbilitySystemComponent->InitStats(AttributeSetClass, InitialStatsTable);
		}
	}
}

TObjectPtr<ACharacter> AAncientKingCharacter::GetAttackTarget()
{
	if (const AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (const UBlackboardComponent* Blackboard = AIController->GetBlackboardComponent())
		{
			if (ACharacter* TargetCharacter = Cast<ACharacter>(Blackboard->GetValueAsObject(TEXT("TargetActor"))))
			{
				return TargetCharacter;
			}
		}
	}
	
	return nullptr;
}