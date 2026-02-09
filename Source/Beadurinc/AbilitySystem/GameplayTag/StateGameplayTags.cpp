#include "StateGameplayTags.h"

namespace StateGameplayTags
{
	/** GameplayTag generating macro */
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_ComboLocked, "State.ComboLocked", "A state where a player's combo attack is blocked");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_BlockingLocked, "State.BlockingLocked", "A state where a player's blocking ability is blocked");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_RollingLocked, "State.RollingLocked", "A state where a player's rolling ability is blocked");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Parry, "State.Parry", "A state that a player can parry attacks from in front");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Blocking, "State.Blocking", "A state that a player can block attacks from in front");
	UE_DEFINE_GAMEPLAY_TAG_COMMENT(State_Invincible, "State.Invincible", "A state that a player immune to damage");
}