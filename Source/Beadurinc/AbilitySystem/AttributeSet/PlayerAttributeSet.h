// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "PlayerAttributeSet.generated.h"

// Declare delegate to subscribe health changes
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FPlayAttributeEvent, float, EffectMagnitude, float, NewValue);

/**
 * 
 */
UCLASS()
class BEADURINC_API UPlayerAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
	
public:
	
	/// Called after attribute value is modified. Allows us to define custom logic after modification. 
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	
	UPROPERTY()
	FPlayAttributeEvent OnHealthChanged;
	
	UPROPERTY()
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UPlayerAttributeSet, Health);
};
