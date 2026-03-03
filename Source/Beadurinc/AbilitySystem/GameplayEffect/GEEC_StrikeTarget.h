// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "GEEC_StrikeTarget.generated.h"

/**
 * GameplayEffectExecutionCalculation: Used for defining complex logics or calculations when GameplayEffect is applied.
 */
UCLASS()
class BEADURINC_API UGEEC_StrikeTarget : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()
	
public:
	UGEEC_StrikeTarget();
	
	/** Override the execution logic */
	virtual void Execute_Implementation
	(
		const FGameplayEffectCustomExecutionParameters& ExecutionParams,
		FGameplayEffectCustomExecutionOutput& OutExecutionOutput
	) const override;
};
