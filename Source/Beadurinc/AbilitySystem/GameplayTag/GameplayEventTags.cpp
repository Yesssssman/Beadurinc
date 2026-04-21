#include "GameplayEventTags.h"

namespace GameplayEventTags
{
	/** GameplayTag generating macro */
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( Event_Combat_Hit,		"Event.Combat.Hit",		"Triggers hit react gameplay ability"	);
	UE_DEFINE_GAMEPLAY_TAG_COMMENT( Event_Combat_Parried,	"Event.Combat.Parried",	"Triggers parry react gameplay ability" );
}