// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ANS_SetWarpingLocation.generated.h"

class AFighterCharacter;

UCLASS()
class BEADURINC_API UANS_SetWarpingLocation : public UAnimNotifyState
{
	GENERATED_BODY()
	
protected:
	virtual void NotifyTick
	(
		USkeletalMeshComponent* MeshComp,
		UAnimSequenceBase* Animation,
		float FrameDeltaTime,
		const FAnimNotifyEventReference& EventReference
	) override;
};
