// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/AttributeSet/PlayerAttributeSet.h"
#include "GameplayEffectExtension.h"

void UPlayerAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	/// Compare attribute to check if the modificed attribute is Health 
	if (Data.EvaluatedData.Attribute == GetHealthAttribute())
	{
		OnHealthChanged.Broadcast(Data.EvaluatedData.Magnitude, GetHealth());
	}
}