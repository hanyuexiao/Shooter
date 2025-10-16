// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MenuSystem : ModuleRules
{
	public MenuSystem(ReadOnlyTargetRules Target) : base(Target)
	{
		PrivateDependencyModuleNames.AddRange(new string[] { "MuitiplayerSession", "Inventory", "WebBrowser" });
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { 
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore",
			"EnhancedInput",
			"UMG",
			"OnlineSubsystemSteam",
			"OnlineSubsystem" ,
			"Niagara"
		});

		// 添加这个判断，来处理仅编辑器使用的模块
		if (Target.bBuildEditor == true)
		{
			PrivateDependencyModuleNames.Add("EditorScriptingUtilities");
		}
	}
}