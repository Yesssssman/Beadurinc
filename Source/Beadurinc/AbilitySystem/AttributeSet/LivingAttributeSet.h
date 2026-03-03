// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "LivingAttributeSet.generated.h"

/**
 * A set of variables related to Ability System, allowing them to be
 * run with ability tasks and gameplay effects.
 */
UCLASS()
class BEADURINC_API ULivingAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
public:
	/// Define a data instance for character's maximum health
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxHealth, meta = (AllowPrivateAccess))
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(ULivingAttributeSet, MaxHealth);
	
	/// Define a data instance for character's maximum stamina
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_MaxStamina, meta = (AllowPrivateAccess))
	FGameplayAttributeData MaxStamina;
	ATTRIBUTE_ACCESSORS_BASIC(ULivingAttributeSet, MaxStamina);
	
	/// Define a data instance for character's health
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Health, meta = (AllowPrivateAccess))
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(ULivingAttributeSet, Health);
	
	/// Define a data instance for character's stamina
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing = OnRep_Stamina, meta = (AllowPrivateAccess))
	FGameplayAttributeData Stamina;
	ATTRIBUTE_ACCESSORS_BASIC(ULivingAttributeSet, Stamina);
	
	/// Called before modifying values. Allows developers to validate modified value to clamp within bounds or
	/// to execute some pre-modification logics.
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	
	/// Called after attribute value is modified. Allows developers to define custom logic after modification occurs.
	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;
	
private:
	
	/// Send changes of health attribute to client in multiplay environment
	UFUNCTION()
	virtual void OnRep_MaxHealth(const FGameplayAttributeData& OldData);
	
	/// Send changes of stamina attribute to client in multiplay environment
	UFUNCTION()
	virtual void OnRep_MaxStamina(const FGameplayAttributeData& OldData);
	
	/// Send changes of stamina attribute to client in multiplay environment
	UFUNCTION()
	virtual void OnRep_Health(const FGameplayAttributeData& OldData);
	
	/// Send changes of stamina attribute to client in multiplay environment
	UFUNCTION()
	virtual void OnRep_Stamina(const FGameplayAttributeData& OldData);
	
	/// Required for networking attributes
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
