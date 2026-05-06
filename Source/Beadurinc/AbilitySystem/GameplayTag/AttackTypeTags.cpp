#include "AttackTypeTags.h"

namespace AttackTypeTags
{
	/** GameplayTag generating macro */
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( AttackType_LeftSwing,	"AttackType.LeftSwing",		"Attacks swing from right to left" );
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( AttackType_RightSwing,	"AttackType.RightSwing",	"Attacks swing from left to right" );
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( AttackType_Pierce,		"AttackType.Pierce",		"Attacks that penetrates blocking" );
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( AttackType_LowAttack,	"AttackType.LowAttack",		"Attacks that ignore blocking"	   );
}