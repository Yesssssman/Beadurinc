// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Beadurinc : ModuleRules
{
	public Beadurinc(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"GameplayAbilities",
			"GameplayTasks",
			"GameplayTags"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"Beadurinc",
			"Beadurinc/Variant_Platforming",
			"Beadurinc/Variant_Platforming/Animation",
			"Beadurinc/Variant_Combat",
			"Beadurinc/Variant_Combat/AI",
			"Beadurinc/Variant_Combat/Animation",
			"Beadurinc/Variant_Combat/Gameplay",
			"Beadurinc/Variant_Combat/Interfaces",
			"Beadurinc/Variant_Combat/UI",
			"Beadurinc/Variant_SideScrolling",
			"Beadurinc/Variant_SideScrolling/AI",
			"Beadurinc/Variant_SideScrolling/Gameplay",
			"Beadurinc/Variant_SideScrolling/Interfaces",
			"Beadurinc/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
