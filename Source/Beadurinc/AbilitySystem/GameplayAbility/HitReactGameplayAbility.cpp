// Fill out your copyright notice in the Description page of Project Settings.

#include "AbilitySystem/GameplayAbility/HitReactGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/GameplayEffect/GE_HealthDamage.h"
#include "AbilitySystem/GameplayEffect/GE_StaminaDamage.h"
#include "AbilitySystem/GameplayTag/AbilityTags.h"
#include "AbilitySystem/GameplayTag/DataTags.h"
#include "AbilitySystem/GameplayTag/GameplayEventTags.h"
#include "Actor/Character/FighterCharacter.h"
#include "Animation/Metadata/AttackMetaData.h"

void UHitReactGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	AFighterCharacter* OwnerCharacter = Cast<AFighterCharacter>(ActorInfo->AvatarActor.Get());
	AFighterCharacter* Attacker = Cast<AFighterCharacter>(const_cast<AActor*>(TriggerEventData->Instigator.Get()));
	UAbilitySystemComponent* OwnerACS = ActorInfo->AbilitySystemComponent.Get();
	
	if (!OwnerCharacter || !OwnerACS)
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}
	
	FGameplayCueParameters CueParams;
	FGameplayEffectContextHandle Context = OwnerACS->MakeEffectContext();
	
	// Fill Cue parameters
	CueParams.Instigator = Attacker;
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
					OwnerACS->PlayMontage(this, ActivationInfo, ParryMontage, 1.0F);
					Parried = true;
				}
				else
				{
					// Play a default block animation when no parry animation found 
					if (OnBlock) OwnerACS->PlayMontage(this, ActivationInfo, OnBlock, 1.0F);
				}
			}
		}
		else
		{
			if (OnBlock) OwnerACS->PlayMontage(this, ActivationInfo, OnBlock, 1.0F);
		}
		
		// Plays gameplay cue for block
		OwnerACS->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.MeleeBlock")), CueParams);
		
		// Applying stamina deflation
		// Sets up GE context
		FGameplayEffectContextHandle GEContext = OwnerACS->MakeEffectContext();
		GEContext.AddInstigator(Attacker, Attacker);
		GEContext.AddSourceObject(Attacker->GetWeaponActor());
		
		// Create GE spec handler to apply damage
		FGameplayEffectSpecHandle SpecHandle = OwnerACS->MakeOutgoingSpec(UGE_StaminaDamage::StaticClass(), 1.0F,GEContext);
		
		if (SpecHandle.IsValid())
		{
			// Apply x0.15 stamina deflation when parried
			SpecHandle.Data.Get()->SetSetByCallerMagnitude(DataTags::DataTag_Stamina, -Attacker->GetWeaponActor()->GetStaminaDamage() * (Parried ? 0.15F : 1.0F));
			OwnerACS->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			
			if (const ULivingAttributeSet* LivingAttributes = Cast<ULivingAttributeSet>(OwnerACS->GetAttributeSet(ULivingAttributeSet::StaticClass())))
			{
				// Plays a stagger animation caused by no stamina if the owner hasn't parried the incomed attack
				if (!Parried && Stagger && LivingAttributes->GetStamina() <= 0.0F)
				{
					OwnerACS->PlayMontage(this, ActivationInfo, Stagger, 1.0F);
				}
			}
		}
	}
	else
	{
		if (OnHurt) OwnerCharacter->PlayAnimMontage(OnHurt);
		// Plays gameplay cue for hurt
		OwnerACS->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.MeleeHurt")), CueParams);
		
		// Applying damage
		// Sets up GE context
		FGameplayEffectContextHandle GEContext = OwnerACS->MakeEffectContext();
		GEContext.AddInstigator(Attacker, Attacker);
		GEContext.AddSourceObject(Attacker->GetWeaponActor());
		
		// Create GE spec handler to apply stamina & health damage
		FGameplayEffectSpecHandle SpecHandleHealth = OwnerACS->MakeOutgoingSpec(UGE_HealthDamage::StaticClass(), 1.0F, GEContext);
		FGameplayEffectSpecHandle SpecHandleStamina = OwnerACS->MakeOutgoingSpec(UGE_StaminaDamage::StaticClass(), 1.0F, GEContext);
		
		if (SpecHandleHealth.IsValid())
		{
			SpecHandleHealth.Data.Get()->SetSetByCallerMagnitude(DataTags::DataTag_Health, -Attacker->GetWeaponActor()->GetHealthDamage());
			OwnerACS->ApplyGameplayEffectSpecToSelf(*SpecHandleHealth.Data.Get());
		}
		
		if (SpecHandleStamina.IsValid())
		{
			SpecHandleStamina.Data.Get()->SetSetByCallerMagnitude(DataTags::DataTag_Stamina, -Attacker->GetWeaponActor()->GetStaminaDamage() * StaminaDamageAttenuation);
			OwnerACS->ApplyGameplayEffectSpecToSelf(*SpecHandleStamina.Data.Get());
			
			if (const ULivingAttributeSet* LivingAttributes = Cast<ULivingAttributeSet>(OwnerACS->GetAttributeSet(ULivingAttributeSet::StaticClass())))
			{
				// Plays a stagger animation caused by no stamina if the owner hasn't parried the incomed attack
				if (StaggerOnHealthDamage && Stagger && LivingAttributes->GetStamina() <= 0.0F)
				{
					OwnerACS->PlayMontage(this, ActivationInfo, Stagger, 1.0F);
					OwnerACS->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.Stagger")), CueParams);
				}
			}
		}
	}
	
	// Apply Hit Stop if the interacting actors are fighters
	Attacker->AddHitActor(OwnerCharacter);
	Attacker->HitStopForTime(HitStop);
	
	// Payback the stamina damage when succeeded at parrying
	if (Parried && Attacker->GetAbilitySystemComponent())
	{
		// Activate parry react ability in opponent
		FGameplayEventData EventContext;
		
		// Fill common parameters
		EventContext.Instigator = OwnerCharacter;
		EventContext.OptionalObject = OwnerCharacter->GetWeaponActor();
		EventContext.Target = Attacker;
		
		// Parry damage formular = (attacker stamina damage + owner stamina damage) / 4
		EventContext.EventMagnitude = -(OwnerCharacter->GetWeaponActor()->GetStaminaDamage() + Attacker->GetWeaponActor()->GetStaminaDamage()) * 0.25F;
		
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Attacker, GameplayEventTags::Event_Combat_Parried, EventContext);
	}
	
	// Makes the actor look at the attacker
	FVector TowardAttacker = TriggerEventData->Instigator.Get()->GetActorLocation() - OwnerCharacter->GetActorLocation();
	// Get a rotator that makes actor looking at the instigator by OwnerActor -> Instigator vector
	FRotator RotatorLookAtAttacker = FRotator(0.0F, TowardAttacker.Rotation().Yaw, 0.0F);
	
	OwnerCharacter->SetActorRotation(RotatorLookAtAttacker);
	
	// Apply stamina gen cooldown
	OwnerCharacter->ApplyStaminaRegenCooldown();
	
	// End ability as soon as triggered
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
