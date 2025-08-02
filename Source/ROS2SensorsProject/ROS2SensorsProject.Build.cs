using UnrealBuildTool;

public class ROS2SensorsProject : ModuleRules
{
	public ROS2SensorsProject(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "ChaosVehicles", "PhysicsCore",
			"CinematicCamera", "CineCameraSceneCapture",
			"rclUE"
		});
	}
}
