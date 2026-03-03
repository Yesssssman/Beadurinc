// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AttributeSet/LivingAttributeSet.h"
#include "GameplayEffectExtension.h"
#include "Net/UnrealNetwork.h"

void ULivingAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// Specifying replication notify: This tells UE to track to sync values from server to clients
	DOREPLIFETIME_CONDITION_NOTIFY(ULivingAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULivingAttributeSet, Stamina, COND_None, REPNOTIFY_Always);
	
	// COND_InitialOnly: Data that clamps other data to maximum will be replicated only once at the initialization
	DOREPLIFETIME_CONDITION_NOTIFY(ULivingAttributeSet, MaxHealth, COND_InitialOnly, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(ULivingAttributeSet, MaxStamina, COND_InitialOnly, REPNOTIFY_Always);
}

void ULivingAttributeSet::OnRep_Health(const FGameplayAttributeData& OldData)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULivingAttributeSet, MaxHealth, OldData);
}

void ULivingAttributeSet::OnRep_MaxHealth(const FGameplayAttributeData& OldData)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULivingAttributeSet, MaxStamina, OldData);
}

void ULivingAttributeSet::OnRep_MaxStamina(const FGameplayAttributeData& OldData)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULivingAttributeSet, Health, OldData);
}

void ULivingAttributeSet::OnRep_Stamina(const FGameplayAttributeData& OldData)
{
	GAMEPLAYATTRIBUTE_REPNOTIFY(ULivingAttributeSet, Stamina, OldData);
}

void ULivingAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	// Clamp values to 0 ~ maximum
	// This blocks GameplayEffect
	if (Attribute == GetHealthAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0F, GetMaxHealth());
	}
	else if (Attribute == GetStaminaAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0F, GetMaxStamina());
	}
}

void ULivingAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	/// Compare attribute to check if the modificed attribute is Health 
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		SetHealth(FMath::Clamp(GetHealth(), 0.0F, GetMaxHealth()));
	}
	else if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		SetStamina(FMath::Clamp(GetStamina(), 0.0F, GetMaxStamina()));
	}
}