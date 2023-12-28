// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MPShooter : ModuleRules
{
	public MPShooter(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[] { "GameplayAbilities", "GameplayTags", "GameplayTasks", "NetCore", "AIModule"});
		
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput" });
	}
}
