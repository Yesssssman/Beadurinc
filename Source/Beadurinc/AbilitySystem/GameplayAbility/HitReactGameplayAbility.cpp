// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/GameplayAbility/HitReactGameplayAbility.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "Actor/Character/FighterCharacter.h"

void UHitReactGameplayAbility::ActivateAbility
(
	const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo,
	const FGameplayEventData* TriggerEventData
)
{
	AFighterCharacter* OwnerCharacter = Cast<AFighterCharacter>(CurrentActorInfo->AvatarActor.Get());
	UAbilitySystemComponent* OwnerACS = ActorInfo->AbilitySystemComponent.Get();
	
	if (OwnerCharacter && OwnerACS)
	{
		FGameplayCueParameters CueParams;
		FGameplayEffectContextHandle Context = OwnerACS->MakeEffectContext();
		
		// Fill Cue parameters
		CueParams.Instigator = const_cast<AActor*>(TriggerEventData->Instigator.Get());
		CueParams.RawMagnitude = TriggerEventData->EventMagnitude;
		
		// Convert collider hit point => cue param spawn location
		Context.AddHitResult(*TriggerEventData->ContextHandle.GetHitResult());
		CueParams.EffectContext = Context;
		
		// When blocking activated
		if (OwnerACS->HasMatchingGameplayTag(StateGameplayTags::State_Blocking))
		{
			if (OnBlock) OwnerCharacter->PlayAnimMontage(OnBlock);
			
			// Plays gameplay cue for block
			OwnerACS->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.MeleeBlock")), CueParams);
		}
		else
		{
			if (OnHurt) OwnerCharacter->PlayAnimMontage(OnHurt);
			// Plays gameplay cue for hurt
			OwnerACS->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.MeleeHurt")), CueParams);
		}
	}
}
