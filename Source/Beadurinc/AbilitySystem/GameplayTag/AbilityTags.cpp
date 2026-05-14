#include "AbilityTags.h"

namespace AbilityTags
{
	/** GameplayTag generating macro */
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( Ability_ComboAttack,	"Ability.ComboAttack",	"An ability for playing successive weapon swings"			);
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( Ability_BlockParry,		"Ability.BlockParry",	"An ability for blocking or parrying enemy attacks"			);
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( Ability_Roll,			"Ability.Roll",			"An ability for rolling ground with short invincible time"	);
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( Ability_AI_Block,		"Ability.AI.Block",		"Trigger tag the BT sends to activate an AI block ability"	);
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( Ability_AI_Parry,		"Ability.AI.Parry",		"Trigger tag the BT sends to activate an AI parry ability"	);
}