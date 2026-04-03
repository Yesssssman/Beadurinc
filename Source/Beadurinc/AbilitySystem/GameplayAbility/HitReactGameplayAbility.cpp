// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/GameplayAbility/HitReactGameplayAbility.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/GameplayTag/AbilityTags.h"
#include "Actor/Character/FighterCharacter.h"
#include "Animation/Metadata/AttackMetaData.h"

void UHitReactGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	AFighterCharacter* OwnerCharacter = Cast<AFighterCharacter>(ActorInfo->AvatarActor.Get());
	UAbilitySystemComponent* OwnerACS = ActorInfo->AbilitySystemComponent.Get();
	
	if (!OwnerCharacter || !OwnerACS)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}
	
	FGameplayCueParameters CueParams;
	FGameplayEffectContextHandle Context = OwnerACS->MakeEffectContext();
	
	// Fill Cue parameters
	CueParams.Instigator = const_cast<AActor*>(TriggerEventData->Instigator.Get());
	CueParams.RawMagnitude = TriggerEventData->EventMagnitude;
	
	// Convert collider hit point => cue param spawn location
	Context.AddHitResult(*TriggerEventData->ContextHandle.GetHitResult());
	CueParams.EffectContext = Context;
	
	// Save whether the attack was parried
	bool Parried = false;
	
	// When blocking activated
	if (OwnerACS->HasMatchingGameplayTag(StateGameplayTags::State_Blocking))
	{
		// Check parrying tag
		if (OwnerACS->HasMatchingGameplayTag(StateGameplayTags::State_Parry))
		{
			if (const UAttackMetaData* MetaData = Cast<UAttackMetaData>(TriggerEventData->OptionalObject2))
			{
				if (UAnimMontage* ParryMontage = *OnParry.Find(MetaData->AttackTypeTag))
				{
					OwnerCharacter->PlayAnimMontage(ParryMontage);
					Parried = true;
				}
				else
				{
					// Play a default block animation when no parry animation found 
					if (OnBlock) OwnerCharacter->PlayAnimMontage(OnBlock);
				}
			}
		}
		else
		{
			if (OnBlock) OwnerCharacter->PlayAnimMontage(OnBlock);
		}
		
		// Plays gameplay cue for block
		OwnerACS->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.MeleeBlock")), CueParams);
	}
	else
	{
		if (OnHurt) OwnerCharacter->PlayAnimMontage(OnHurt);
		// Plays gameplay cue for hurt
		OwnerACS->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.MeleeHurt")), CueParams);
	}
	
	// Apply Hit Stop if the interacting actors are fighters
	if (AFighterCharacter* Attacker = Cast<AFighterCharacter>(CueParams.Instigator))
	{
		Attacker->AddHitActor(OwnerCharacter);
		Attacker->HitStopForTime(HitStop);
		
		if (Parried && Attacker->GetAbilitySystemComponent())
		{
			// Add "Parried" state tag to play short parry stagger animation to attacker
			Attacker->GetAbilitySystemComponent()->AddLooseGameplayTag(StateGameplayTags::State_Parried);
		}
	}
	
	// Makes the actor look at attacker (for monsters)
	if (LookAttacker)
	{
		FVector TowardAttacker = TriggerEventData->Instigator.Get()->GetActorLocation() - OwnerCharacter->GetActorLocation();
		// Get a rotator that makes actor looking at the instigator by OwnerActor -> Instigator vector
		FRotator RotatorLookAtAttacker = FRotator(0.0F, TowardAttacker.Rotation().Yaw, 0.0F);
		
		OwnerCharacter->SetActorRotation(RotatorLookAtAttacker);
	}
	
	// End ability as soon as triggered
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
