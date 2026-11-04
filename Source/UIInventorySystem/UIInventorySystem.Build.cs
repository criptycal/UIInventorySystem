// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class UIInventorySystem : ModuleRules
{
	public UIInventorySystem(ReadOnlyTargetRules Target) : base(Target)
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
			"SlateCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"UIInventorySystem",
			"UIInventorySystem/Variant_Platforming",
			"UIInventorySystem/Variant_Platforming/Animation",
			"UIInventorySystem/Variant_Combat",
			"UIInventorySystem/Variant_Combat/AI",
			"UIInventorySystem/Variant_Combat/Animation",
			"UIInventorySystem/Variant_Combat/Gameplay",
			"UIInventorySystem/Variant_Combat/Interfaces",
			"UIInventorySystem/Variant_Combat/UI",
			"UIInventorySystem/Variant_SideScrolling",
			"UIInventorySystem/Variant_SideScrolling/AI",
			"UIInventorySystem/Variant_SideScrolling/Gameplay",
			"UIInventorySystem/Variant_SideScrolling/Interfaces",
			"UIInventorySystem/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
