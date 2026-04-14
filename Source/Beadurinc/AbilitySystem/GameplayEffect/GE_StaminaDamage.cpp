// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/GameplayEffect/GE_StaminaDamage.h"

#include "AbilitySystem/AttributeSet/LivingAttributeSet.h"
#include "AbilitySystem/GameplayTag/DataTags.h"

UGE_StaminaDamage::UGE_StaminaDamage()
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
