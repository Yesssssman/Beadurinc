#include "BeadurincHUD.h"

/**
 * Initialize the player character
 * Called on PlayerController starts possessing character
 */
void ABeadurincHUD::InitPlayer(const TObjectPtr<APlayerCharacter> SpawnedPlayer)
{
	PlayerCharacter = SpawnedPlayer;
}