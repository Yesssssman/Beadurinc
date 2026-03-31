// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/ANS_SetWarpingLocation.h"

#include "RootMotionModifier.h"
#include "Actor/Character/FighterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "MotionWarpingComponent.h"

void UANS_SetWarpingLocation::NotifyTick
(
	USkeletalMeshComponent* MeshComp,
	UAnimSequenceBase* Animation,
	float FrameDeltaTime,
	const FAnimNotifyEventReference& EventReference
)
{
	if (AFighterCharacter* Owner = Cast<AFighterCharacter>(MeshComp->GetOwner()))
	{
		// Checks the validity of an attack target
		if (IsValid(Owner->GetAttackTarget()))
		{
			// Update the location of Motion Warping target
			FMotionWarpingTarget MotionWarpingTarget;
			MotionWarpingTarget.Location = Owner->GetAttackTarget()->GetActorLocation();
	
			UCapsuleComponent* OwnerCapsuleComponent = Owner->GetCapsuleComponent();
			UCapsuleComponent* TargetCapsuleComponent = Owner->GetAttackTarget()->GetCapsuleComponent();
	
			// Push the motion warping target location toward myself, by the distance that
			// equals to the radius of target's capsule component (if exists)
			if (OwnerCapsuleComponent && TargetCapsuleComponent)
			{
				FVector FromTargetToMyself = Owner->GetActorLocation() - Owner->GetAttackTarget()->GetActorLocation();
				FromTargetToMyself.Normalize(0.05F);
				FromTargetToMyself *= OwnerCapsuleComponent->GetScaledCapsuleRadius() + TargetCapsuleComponent->GetScaledCapsuleRadius();
				MotionWarpingTarget.Location += FromTargetToMyself;
			}
	
			MotionWarpingTarget.Name = TEXT("AttackTarget");
			Owner->GetMotionWarpingComponent()->AddOrUpdateWarpTarget(MotionWarpingTarget);
		}
	}
}