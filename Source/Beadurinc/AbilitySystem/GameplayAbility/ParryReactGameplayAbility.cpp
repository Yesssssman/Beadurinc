#include "AbilitySystem/GameplayAbility/ParryReactGameplayAbility.h"

#include "AbilitySystem/GameplayEffect/GE_StaminaDamage.h"
#include "AbilitySystem/GameplayTag/DataTags.h"
#include "Actor/Character/FighterCharacter.h"
#include "Animation/Metadata/AttackMetaData.h"

void UParryReactGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo)) return;
	
	AFighterCharacter* FighterCharacter = Cast<AFighterCharacter>(ActorInfo->AvatarActor);
	
	if (!FighterCharacter) return;
	
	// Reflect stamina damage when parried
	FGameplayEffectContextHandle GEContext = ActorInfo->AbilitySystemComponent->MakeEffectContext();
	AActor* MutableInstigator = const_cast<AActor*>(TriggerEventData->Instigator.Get());
	GEContext.AddInstigator(MutableInstigator, MutableInstigator);
	GEContext.AddSourceObject(TriggerEventData->OptionalObject);
	
	FGameplayEffectSpecHandle SpecHandle = ActorInfo->AbilitySystemComponent->MakeOutgoingSpec(UGE_StaminaDamage::StaticClass(), 1.0F,GEContext);
	
	if (SpecHandle.IsValid())
	{
		SpecHandle.Data.Get()->SetSetByCallerMagnitude(DataTags::DataTag_Stamina, TriggerEventData->EventMagnitude);
		ActorInfo->AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
	}
	
	if (const ULivingAttributeSet* LivingAttributes = Cast<ULivingAttributeSet>(ActorInfo->AbilitySystemComponent->GetAttributeSet(ULivingAttributeSet::StaticClass())))
	{
		// Plays a stagger animation caused by no stamina if the owner hasn't parried the incomed attack
		if (LivingAttributes->GetStamina() <= 0.0F)
		{
			ActorInfo->AbilitySystemComponent->PlayMontage(this, ActivationInfo, Stagger, 1.0F);
			
			FGameplayCueParameters CueParams;
			CueParams.Instigator = ActorInfo->AvatarActor;
			ActorInfo->AbilitySystemComponent->ExecuteGameplayCue(FGameplayTag::RequestGameplayTag(FName("GameplayCue.Stagger")), CueParams);
			return;
		}
	}
	
	UAnimMontage* CurrentPlayingMontage = FighterCharacter->GetCurrentMontage();
	if (!CurrentPlayingMontage) return;
	
	for (UAnimMetaData* ComboMetaData : CurrentPlayingMontage->GetMetaData())
	{
		UAttackMetaData* AttackAnimMetaData = Cast<UAttackMetaData>(ComboMetaData);
		if (!AttackAnimMetaData || !AttackAnimMetaData->OnParried) continue;
		
		ActorInfo->AbilitySystemComponent->PlayMontage(this, ActivationInfo, AttackAnimMetaData->OnParried, 1.0F);
	}
}
