#include "AbilityTags.h"

namespace AbilityTags
{
	/** GameplayTag generating macro */
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( Ability_ComboAttack,	"Ability.ComboAttack",	"An ability for playing successive weapon swings"			);
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( Ability_BlockParry,		"Ability.BlockParry",	"An ability for blocking or parrying enemy attacks"			);
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( Ability_Roll,			"Ability.Roll",			"An ability for rolling ground with short invincible time"	);
}