// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FighterCharacter.h"
#include "AncientKingCharacter.generated.h"

class UAbilitySystemComponent;
class ULivingAttributeSet;

UCLASS()
class BEADURINC_API AAncientKingCharacter : public AFighterCharacter
{
	GENERATED_BODY()
	
	/** Attribute Set Class */
	UPROPERTY(EditDefaultsOnly, Category="Gameplay Abilities")
	TSubclassOf<UAttributeSet> AttributeSetClass;
	
	/** Data Table for initializing AttributeSet data */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gameplay Abilities", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UDataTable> InitialStatsTable;
	
public:
	
	virtual void BeginPlay() override;
	
	// Sets default values for this character's properties
	AAncientKingCharacter();
};
