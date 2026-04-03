// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "CheckAndPlayParriedAnimNotify.generated.h"

UCLASS()
class BEADURINC_API UCheckAndPlayParriedAnimNotify : public UAnimNotifyState
{
	GENERATED_BODY()

public:

	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

private:

	/** An animation montage to play when the attack get parried */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat", meta = (AllowPrivateAccess = true))
	TObjectPtr<UAnimMontage> Parried;
};