// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/AnimNotify/ANS_SetWarpingLocation.h"

#include "RootMotionModifier.h"
#include "Actor/Character/FighterCharacter.h"
#include "Components/CapsuleComponent.h"
#include "MotionWarpingComponent.h"

void UANS_SetWarpingLocation::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	AFighterCharacter* Owner = Cast<AFighterCharacter>(MeshComp->GetOwner());
	if (!Owner) return;
	// Checks the validity of an attack target
	if (!IsValid(Owner->GetAttackTarget())) return;
	
	// Update the location of Motion Warping target
	FMotionWarpingTarget MotionWarpingTarget;
	float Distance = Distancing;
	
	UCapsuleComponent* OwnerCapsuleComponent = Owner->GetCapsuleComponent();
	UCapsuleComponent* TargetCapsuleComponent = Owner->GetAttackTarget()->GetCapsuleComponent();

	// Add the radius of each actor's capsule component to distance
	if (OwnerCapsuleComponent) Distance += OwnerCapsuleComponent->GetScaledCapsuleRadius();
	if (TargetCapsuleComponent) Distance += TargetCapsuleComponent->GetScaledCapsuleRadius();
	
	FVector ToTarget = Owner->GetAttackTarget()->GetActorLocation() - Owner->GetActorLocation();
	
	// Calculate the length of the vector toward actor -> target, then subtract the distancing while keeping the distance
	// not to be a negative so that the motion warping destination is not behind the owner actor.
	double Length = ToTarget.Length();
	double WarpDistance = FMath::Max(Length - Distance, 0.001F);
	
	// Apply the distance to the (owner -> target) vector
	ToTarget *= (WarpDistance / Length);
	
	DrawDebugSphere(Owner->GetWorld(), Owner->GetActorLocation() + ToTarget, 
		16, 16, FColor::Green, false, -1, 0, 2.0F);
	
	MotionWarpingTarget.Location = Owner->GetActorLocation() + ToTarget;
	MotionWarpingTarget.Name = TEXT("AttackTarget");
	Owner->GetMotionWarpingComponent()->AddOrUpdateWarpTarget(MotionWarpingTarget);
}