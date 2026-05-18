#include "AbilitySystem/GameplayEffect/GE_StaminaModifier.h"
#include "AbilitySystem/AttributeSet/LivingAttributeSet.h"
#include "AbilitySystem/GameplayTag/DataTags.h"

UGE_StaminaModifier::UGE_StaminaModifier()
{
	// Sets up GE applying logic modules
	DurationPolicy = EGameplayEffectDurationType::Instant;

	FGameplayModifierInfo Modifier;
	Modifier.Attribute = ULivingAttributeSet::GetStaminaAttribute();
	Modifier.ModifierOp = EGameplayModOp::Additive;

	FSetByCallerFloat SetByCaller;
	SetByCaller.DataTag = DataTags::DataTag_Stamina;
	Modifier.ModifierMagnitude = FGameplayEffectModifierMagnitude(SetByCaller);

	Modifiers.Add(Modifier);
}
