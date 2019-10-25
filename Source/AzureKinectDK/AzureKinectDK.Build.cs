// Copyright (c) Cristian Gómez. All rights reserved.e of Project Settings.

using UnrealBuildTool;
using System.IO;
using System;

public class AzureKinectDK : ModuleRules
{
	public AzureKinectDK(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[] {  });

        string programFilesPath = Environment.GetFolderPath(Environment.SpecialFolder.ProgramFiles);
        PublicIncludePaths.Add(Path.Combine(programFilesPath, "Azure Kinect SDK v1.3.0/sdk/include"));
        PublicIncludePaths.Add(Path.Combine(programFilesPath, "Azure Kinect Body Tracking SDK/sdk/include"));

        PublicAdditionalLibraries.Add(Path.Combine(programFilesPath, "Azure Kinect SDK v1.3.0/sdk/windows-desktop/amd64/release/lib/k4a.lib"));
        PublicAdditionalLibraries.Add(Path.Combine(programFilesPath, "Azure Kinect SDK v1.3.0/sdk/windows-desktop/amd64/release/lib/k4arecord.lib"));
        PublicAdditionalLibraries.Add(Path.Combine(programFilesPath, "Azure Kinect Body Tracking SDK/sdk/windows-desktop/amd64/release/lib/k4abt.lib"));
    }
}
