#pragma once

#include "NativeGameplayTags.h"

namespace StateGameplayTags
{
	/** GameplayTag definition macro */
	BEADURINC_API UE_DECLARE_GAMEPLAY_TAG_EXTERN( State_ComboLocked	   );
	BEADURINC_API UE_DECLARE_GAMEPLAY_TAG_EXTERN( State_Blocking	   );
	BEADURINC_API UE_DECLARE_GAMEPLAY_TAG_EXTERN( State_BlockingLocked );
	BEADURINC_API UE_DECLARE_GAMEPLAY_TAG_EXTERN( State_RollingLocked  );
	BEADURINC_API UE_DECLARE_GAMEPLAY_TAG_EXTERN( State_Parry		   );
	BEADURINC_API UE_DECLARE_GAMEPLAY_TAG_EXTERN( State_Parried		   ); // Set ONLY once when get parried
}
