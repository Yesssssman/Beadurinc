#include "MonsterCharacter.h"
#include "Net/UnrealNetwork.h"

AMonsterCharacter::AMonsterCharacter()
{
	bReplicates = true;
}

void AMonsterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME( AMonsterCharacter, BTState );
}