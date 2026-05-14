#include "StateGameplayTags.h"

namespace StateGameplayTags
{
	/** GameplayTag generating macro */
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( State_ComboLocked,				"State.ComboLocked",				"A state where a player's combo attack is blocked"							 );
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( State_BlockingLocked,			"State.BlockingLocked",				"A state where a player's blocking ability is blocked"						 );
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( State_RollingLocked,			"State.RollingLocked",				"A state where a player's rolling ability is blocked"						 );
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( State_Parry,					"State.Parry",						"A state that a player can parry attacks from in front"						 );
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( State_Stunned,					"State.Stunned",					"A state for actor being hurt. Used in triggering block & parry abilities."	 );
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( State_Attacking,				"State.Attacking",					"A state for playing offensive attack anim montages. Used in Behavior Tree." );
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( State_Blocking,					"State.Blocking",					"A state that a player can block attacks from in front"						 );
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( State_StaminaRegenCooldown,		"State.StaminaRegenCooldown",		"A state that stamina regeneration is blocked"								 );
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( State_LockOnRotationSuppressed,	"State.LockOnRotationSuppressed",	"A state where the actor should not rotate to face the lock-on target"		 );
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( State_HoldComboSequence,		"State.HoldComboSequence",			"A state that keeps combo counter (both for player & AI)"					 );
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( State_VulnerableToExecution,	"State.VulnerableToExecution",		"A state that allows to be executed by finisher moves"						 );
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( State_LowAttackDodgeable,		"State.LowAttackDodgeable",			"A state that being invulnerable to AttackTypeTags::LowAttack"				 );
}