#include "MonsterCharacter.h"

#include "AbilitySystemComponent.h"
#include "Net/UnrealNetwork.h"

AMonsterCharacter::AMonsterCharacter()
{
	bReplicates = true;
}

void AMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent && HasAuthority())
	{
		if (IsValid(AIBlockAbility))
		{
			FGameplayAbilitySpec Spec(AIBlockAbility, 1, INDEX_NONE, this);
			AbilitySystemComponent->GiveAbility(Spec);
		}

		if (IsValid(AIParryAbility))
		{
			FGameplayAbilitySpec Spec(AIParryAbility, 1, INDEX_NONE, this);
			AbilitySystemComponent->GiveAbility(Spec);
		}
	}
}

void AMonsterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME( AMonsterCharacter, BTState );
}