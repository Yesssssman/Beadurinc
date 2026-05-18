#include "AbilitySystem/GameplayEffect/GE_HealthModifier.h"
#include "AbilitySystem/AttributeSet/LivingAttributeSet.h"
#include "AbilitySystem/GameplayTag/DataTags.h"

UGE_HealthModifier::UGE_HealthModifier()
{
	// Sets up GE applying logic modules
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = ULivingAttributeSet::GetHealthAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;

	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = DataTags::DataTag_Health;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);

	Modifiers.Add(Modifier);
}
