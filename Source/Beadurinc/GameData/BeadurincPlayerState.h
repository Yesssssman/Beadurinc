// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "BeadurincPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class BEADURINC_API ABeadurincPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	
	UPROPERTY()
	UAbilitySystemComponent* AbilitySystemComponent;
	
public:

	/** Constructor */
	ABeadurincPlayerState();

	/** Returns Ability Component object **/
	FORCEINLINE virtual UAbilitySystemComponent* GetAbilitySystemComponent() const { return AbilitySystemComponent; };
};
