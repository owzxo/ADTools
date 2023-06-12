// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ADTools : ModuleRules
{
	public ADTools(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core", 
				"Json", 
				"JsonUtilities",
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"MeshDescription", // Generated meshes -> static meshes
				"ViewportInteraction",
				"VREditor",
				
				"Projects",
				"InputCore",
				"EditorFramework",
				"UnrealEd",
				"Sockets", // tcp stuff
				"Networking", // tcp stuff
				"HTTPServer",
				"MaterialEditor",
				"ToolMenus",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"EditorStyle",
				"Json", 
				"JsonUtilities",
				"ViewportInteraction",
				"RHI", // holds definitions of shit like this: GMaxRHIFeatureLevel
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
	}
}
