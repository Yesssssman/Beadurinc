#include "AbilitySystem/GameplayAbility/ExecutionGameplayAbility.h"
#include "Actor/Character/FighterCharacter.h"
#include "ContextualAnimTypes.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"
#include "ContextualAnimSceneActorComponent.h"

bool UExecutionGameplayAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags, const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	AFighterCharacter* OwnerCharacter = Cast<AFighterCharacter>(ActorInfo->AvatarActor.Get());
	if (!OwnerCharacter) return false;
	
	if (ActorInfo->AbilitySystemComponent->HasMatchingGameplayTag(StateGameplayTags::State_ComboLocked)) return false;
	
	AFighterCharacter* TargetCharacter = Cast<AFighterCharacter>(OwnerCharacter->GetAttackTarget());
	if (!TargetCharacter) return false;
	
	// Check GAS tag
	if (!TargetCharacter->GetAbilitySystemComponent()->HasMatchingGameplayTag(StateGameplayTags::State_VulnerableToExecution)) return false;
	
	// Check Distance
	if ((TargetCharacter->GetActorLocation() - OwnerCharacter->GetActorLocation()).SquaredLength() > Distance * Distance) return false; 
	
	// Check Angle
	FVector ToOwnerDir = (OwnerCharacter->GetActorLocation() - TargetCharacter->GetActorLocation()).GetSafeNormal();
	FVector TargetLocalDir = TargetCharacter->GetActorTransform().InverseTransformVectorNoScale(ToOwnerDir);
	// Get an angle on XZ plane (right angle to gravity direction) by atan that distinguishes positive and negative angle
	float TargetAngle = FMath::RadiansToDegrees(FMath::Atan2(TargetLocalDir.Y, TargetLocalDir.X));
	
	if (AngleFrom <= AngleTo)
	{
		return (TargetAngle >= AngleFrom && TargetAngle <= AngleTo);
	}
	else
	{
		return (TargetAngle >= AngleFrom || TargetAngle <= AngleTo);
	}
};

void UExecutionGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	AFighterCharacter* OwnerCharacter = Cast<AFighterCharacter>(ActorInfo->AvatarActor.Get());
	
	if (OwnerCharacter && ExecutionAnimation)
	{
		FContextualAnimSceneBindings ContextualAnimBindings(*ExecutionAnimation, 0, 0);
		
		// Bind each actor with role names
		ContextualAnimBindings.BindActorToRole(*OwnerCharacter, FName(TEXT("Executioner")));
		ContextualAnimBindings.BindActorToRole(*OwnerCharacter->GetAttackTarget(), FName(TEXT("Executed")));
		
		// Start CAS with binding info
		OwnerCharacter->GetContextualAnimSceneActorComponent()->StartContextualAnimScene(ContextualAnimBindings);
	};
	
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}