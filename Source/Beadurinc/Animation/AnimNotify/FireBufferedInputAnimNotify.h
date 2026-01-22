// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "FireBufferedInputAnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class BEADURINC_API UFireBufferedInputAnimNotify : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	
	virtual void Notify
	(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		const FAnimNotifyEventReference& EventReference
	) override;
};
