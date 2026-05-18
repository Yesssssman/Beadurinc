#include "BeadurincPlayerState.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AbilityId.h"
#include "AbilitySystem/GameplayEffect/GE_HealthModifier.h"
#include "AbilitySystem/GameplayTag/DataTags.h"

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
			RollAbilitySpecHandle = AbilitySystemComponent->GiveAbility(RollAbilitySpec);
		}
		
		// Give execution ability
		if (ExecutionAbility)
		{
			FGameplayAbilitySpec ExecutionAbilitySpec(ExecutionAbility, 1, static_cast<int32>(EAbilityId::Execution), this);
			AbilitySystemComponent->GiveAbility(ExecutionAbilitySpec);
		}

		// Give heavy attack ability
		if (HeavyAttackAbility)
		{
			FGameplayAbilitySpec HeavyAttackAbilitySpec(HeavyAttackAbility, 1, static_cast<int32>(EAbilityId::Heavy_Attack), this);
			AbilitySystemComponent->GiveAbility(HeavyAttackAbilitySpec);
		}

		// Give self healing ability
		if (HealAbility)
		{
			FGameplayAbilitySpec HeavyAttackAbilitySpec(HealAbility, 1, static_cast<int32>(EAbilityId::Heal), this);
			AbilitySystemComponent->GiveAbility(HeavyAttackAbilitySpec);
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
	
	RemainingHealingPotion = 10;
}

void ABeadurincPlayerState::ConsumeHealingPotion()
{
	RemainingHealingPotion = FMath::Max(RemainingHealingPotion - 1, 0);
	
	const ULivingAttributeSet* LivingAttributeSet = CastChecked<ULivingAttributeSet>(AbilitySystemComponent->GetAttributeSet(ULivingAttributeSet::StaticClass()));
	
	if (LivingAttributeSet)
	{
		FGameplayEffectContextHandle GEContext = AbilitySystemComponent->MakeEffectContext();
		
		// Create GE spec handler to apply stamina & health damage
		FGameplayEffectSpecHandle SpecHandleHealth = AbilitySystemComponent->MakeOutgoingSpec(UGE_HealthModifier::StaticClass(), 1.0F, GEContext);
		
		if (SpecHandleHealth.IsValid())
		{
			SpecHandleHealth.Data.Get()->SetSetByCallerMagnitude(DataTags::DataTag_Health, 120);
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandleHealth.Data.Get());
		}
	}
}