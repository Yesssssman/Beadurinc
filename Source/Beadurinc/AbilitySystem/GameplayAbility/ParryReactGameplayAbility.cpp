#include "AbilitySystem/GameplayAbility/ParryReactGameplayAbility.h"

#include "AbilitySystem/GameplayEffect/GE_StaminaModifier.h"
#include "AbilitySystem/GameplayTag/DataTags.h"
#include "AbilitySystem/GameplayTag/StateGameplayTags.h"
#include "Actor/Character/FighterCharacter.h"
#include "Actor/Character/MonsterCharacter.h"
#include "Animation/Metadata/AttackMetaData.h"

void UParryReactGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		// End ability as soon as triggered
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}
	
	AFighterCharacter* OwnerCharacter = Cast<AFighterCharacter>(ActorInfo->AvatarActor);
	
	if (!OwnerCharacter) return;
	
	// Reflect stamina damage when parried
	FGameplayEffectContextHandle GEContext = ActorInfo->AbilitySystemComponent->MakeEffectContext();
	AActor* MutableInstigator = const_cast<AActor*>(TriggerEventData->Instigator.Get());
	GEContext.AddInstigator(MutableInstigator, MutableInstigator);
	GEContext.AddSourceObject(TriggerEventData->OptionalObject);
	
	FGameplayEffectSpecHandle SpecHandle = ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(UGE_StaminaModifier::StaticClass(), 1.0F,GEContext);
	
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(DataTags::DataTag_Stamina, TriggerEventData->EventMagnitude);
		ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		OwnerCharacter->ApplyStaminaRegenCooldown();
	}
	
	if (const ULivingAttributeSet* LivingAttributes = Cast<ULivingAttributeSet>(ActorInfo->AbilitySystemComponent->GetAttributeSet(ULivingAttributeSet::StaticClass())))
	{
		// Plays a stagger animation caused by no stamina if the owner hasn't parried the incomed attack
		if (Stagger && LivingAttributes->GetStamina() <= 0.0F)
		{
			OwnerCharacter->PlayAnimMontage(Stagger);
			
			FGameplayCueParameters CueParams;
			CueParams.Instigator = ActorInfo->AvatarActor;
			ActorInfo->AbilitySystemComponent->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.Stagger")), CueParams);
			
			// Restore stamina
			GEContext.AddInstigator(OwnerCharacter, OwnerCharacter);
			FGameplayEffectSpecHandle StaminaSpecHandle = ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(UGE_StaminaModifier::StaticClass(), 1.0F,GEContext);
			StaminaSpecHandle.Data.Get()->SetSetByCallerMagnitude(DataTags::DataTag_Stamina, 20.0F);
			ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*StaminaSpecHandle.Data.Get());
			
			// End ability as soon as triggered
			EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
			return;
		}
	}
	
	UAnimMontage* CurrentPlayingMontage = OwnerCharacter->GetCurrentMontage();
	
	if (!CurrentPlayingMontage)
	{
		// End ability as soon as triggered
		EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
		return;
	}
	
	for (UAnimMetaData* ComboMetaData : CurrentPlayingMontage->GetMetaData())
	{
		UAttackMetaData* AttackAnimMetaData = Cast<UAttackMetaData>(ComboMetaData);
		if (!AttackAnimMetaData || !AttackAnimMetaData->OnParried) continue;
		
		OwnerCharacter->PlayAnimMontage(AttackAnimMetaData->OnParried);
	}
	
	// Gives combat feedbacks to each character: owner was offended, situation went bad for owner, good for enemy.
	
	if (AMonsterCharacter* OwnerAsMonster = Cast<AMonsterCharacter>(OwnerCharacter))
	{
		// Negative feedback to owner
		OwnerAsMonster->ModifyBlackboardScore(FName("RetreatScore"), 4, 4);
	}
	
	if (AMonsterCharacter* AttackerAsMonster = Cast<AMonsterCharacter>(const_cast<AActor*>(TriggerEventData->Instigator.Get())))
	{
		// Positive feedback to enemy
		AttackerAsMonster->ModifyBlackboardScore(FName("RetreatScore"), -10, -20);
	}
	
	// End ability as soon as triggered
	EndAbility(Handle, ActorInfo, ActivationInfo, false, false);
}
