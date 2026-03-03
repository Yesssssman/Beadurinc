// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystem/AttributeSet/LivingAttributeSet.h"
#include "GameFramework/PlayerState.h"
#include "BeadurincPlayerState.generated.h"

/**
 * PlayerState: This object exists in both client and server side, including remote clients.
 *				It allows referencing other players to see the states of a specific player.
 *
 * PlayerCharacter vs PlayerState: PlayerCharacter is a transient object that is destroyed
 *								   when the character dies, destroying all data that the player
 *								   have made so far. Data that along with the character's lifecycle
 *								   won't be affected by this(Health, Mana, etc.), while data that
 *								   hold the gameplay progression shouldn't be discarded due to
 *								   a player object is destroyed.
 *								   
 *								   PlayerState provides a permanent and coherent space to save data
 *								   that needs to persist in entire gameplay. In this way, developers
 *								   can keep the old data when player respawning occurs and determine
 *								   which to keep and which to remove. (Player level, achievement
 *								   progressions, etc.)
 */
UCLASS()
class BEADURINC_API ABeadurincPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
	/// Keeping ASC in PlayerState will persist the player data (cooldown, score) after death
	/// and respawn, allowing a player respawns with previous data. 
	UPROPERTY()
	UAbilitySystemComponent* AbilitySystemComponent;
	
	/** Attribute Set Class */
	UPROPERTY(EditDefaultsOnly, Category="Attribute")
	TSubclassOf<UAttributeSet> AttributeSetClass;
	
	/** Gameplay Ability class for Combo Attacks */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	TSubclassOf<UGameplayAbility> ComboAttackAbility;
	
	/** Gameplay Ability class for Blocking */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	TSubclassOf<UGameplayAbility> BlockAbility;
	
	/** Gameplay Ability class for Rolling */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	TSubclassOf<UGameplayAbility> RollAbility;
	
	/** Data Table for initializing AttributeSet data */
	UPROPERTY(EditDefaultsOnly, Category = "Gameplay Abilities")
	TObjectPtr<UDataTable> InitialStatsTable;
	
protected:
	/** Gameplay Abilities Attribute Set */
	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;
	
public:

	/** Constructor */
	ABeadurincPlayerState();

	virtual void BeginPlay() override;
	
	/** Reset all stats to initial values */
	void ResetStats();
	
	/** Returns Ability Component object **/
	FORCEINLINE virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override { return AbilitySystemComponent; };
	
	FORCEINLINE virtual TObjectPtr<UAttributeSet> GetAttributeSet() const { return AttributeSet; };
};
