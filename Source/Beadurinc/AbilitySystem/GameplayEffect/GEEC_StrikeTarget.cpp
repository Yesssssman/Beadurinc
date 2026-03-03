// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GameplayEffect/GEEC_StrikeTarget.h"
#include "AbilitySystem/AttributeSet/LivingAttributeSet.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"
#include "Math/RandomStream.h"

/// Internal struct to handle attribute capture
struct DamageStatics {
	DECLARE_ATTRIBUTE_CAPTUREDEF(Health);
	DECLARE_ATTRIBUTE_CAPTUREDEF(Stamina);

	DamageStatics() {
		// Capture Health and Stamina from the Target
		DEFINE_ATTRIBUTE_CAPTUREDEF(ULivingAttributeSet, Health, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(ULivingAttributeSet, Stamina, Target, false);
	}
};

/// Singleton-pattern getter for the internal attribute capturer struct
static const DamageStatics& GetDamageStatics() {
	static DamageStatics Statics;
	return Statics;
}

UGEEC_StrikeTarget::UGEEC_StrikeTarget()
{
	RelevantAttributesToCapture.Add(GetDamageStatics().HealthDef);
	RelevantAttributesToCapture.Add(GetDamageStatics().StaminaDef);
}

void UGEEC_StrikeTarget::Execute_Implementation
(
	const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	FGameplayEffectCustomExecutionOutput& OutExecutionOutput
) const
{
	UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

	FRandomStream RandomStream;
	RandomStream.Initialize(FMath::Rand());
	
	float RandomFloat = RandomStream.FRand();
	
	// Checks for the Blocking Tag
	bool bIsBlocking = RandomFloat > 0.5F; // TargetASC->HasMatchingGameplayTag(StateGameplayTags::State_Blocking);

	// Extract the original magnitude (inflicts to health)
	float OriginalDamage;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(GetDamageStatics().HealthDef, FAggregatorEvaluateParameters(), OriginalDamage);

	// Copy the original context to Cue params which hands over parameters to GameplayCue
	FGameplayEffectContextHandle Context = ExecutionParams.GetOwningSpec().GetContext();
	FGameplayCueParameters CueParams;
	
	CueParams.EffectContext = Context;
	CueParams.RawMagnitude = OriginalDamage;
	
	// If the hitter taking a blocking stance, deflate stamina and apply blocking gameplay cue
	if (bIsBlocking) {
		float StaminaDamage = OriginalDamage * 0.5f;
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(GetDamageStatics().StaminaProperty, EGameplayModOp::Additive, StaminaDamage));
		TargetASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.MeleeBlock")), CueParams);
	}
	// If not blocking, just apply damage and stuns
	else
	{
		OutExecutionOutput.AddOutputModifier(FGameplayModifierEvaluatedData(GetDamageStatics().HealthProperty, EGameplayModOp::Additive, OriginalDamage));
		TargetASC->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.MeleeHurt")), CueParams);
	}
}