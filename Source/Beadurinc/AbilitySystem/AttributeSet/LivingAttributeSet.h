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
	/** Constructor */
	ULivingAttributeSet();
	
	/** Called before any change is applied to attribute value. That change is transient until GE ends */
	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	
	/** Called before any change is applied to attribute base value */
	virtual void PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const override;
	
	/** Called after attribute value change is confirmed */
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

private:
	
	/** Information about max value of Attributes */
	TMap<FGameplayAttribute, FGameplayAttribute> MaxValues;
	
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
};