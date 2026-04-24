#include "BeadurincPlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AbilityId.h"

ABeadurincPlayerState::ABeadurincPlayerState()
{
	// Create GAS component
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	
	// Replicated = true so client side ASC knows the updated values
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
	
	SetNetUpdateFrequency(100.0F);
}

void ABeadurincPlayerState::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize only in authorized side to let them replicated to clients by networking
	if (HasAuthority())
	{
		// Give combo attack ability
		if (ComboAttackAbility)
		{
			FGameplayAbilitySpec ComboAttackAbilitySpec(ComboAttackAbility, 1, static_cast<int32>(EAbilityId::Combo_Attack), this);
			AbilitySystemComponent->GiveAbility(ComboAttackAbilitySpec);
		}
		
		// Give block ability
		if (BlockAbility)
		{
			FGameplayAbilitySpec BlockAbilitySpec(BlockAbility, 1, static_cast<int32>(EAbilityId::Block), this);
			AbilitySystemComponent->GiveAbility(BlockAbilitySpec);
		}
		
		// Give roll ability
		if (RollAbility)
		{
			FGameplayAbilitySpec RollAbilitySpec(RollAbility, 1, static_cast<int32>(EAbilityId::Roll), this);
			AbilitySystemComponent->GiveAbility(RollAbilitySpec);
		}
		
		if (AttributeSetClass)
		{
			// Create AttributeSet
			UAttributeSet* CreatedAttributeSet = NewObject<UAttributeSet>(this, AttributeSetClass);
			
			AttributeSet = CreatedAttributeSet;
			AbilitySystemComponent->AddAttributeSetSubobject(CreatedAttributeSet);
			
			ResetStats();
		}
	}
}

void ABeadurincPlayerState::ResetStats()
{
	if (InitialStatsTable)
	{
		AbilitySystemComponent->InitStats(AttributeSetClass, InitialStatsTable);
	}
}
