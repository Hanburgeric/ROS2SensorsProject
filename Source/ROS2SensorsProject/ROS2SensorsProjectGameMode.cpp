// Copyright Epic Games, Inc. All Rights Reserved.

#include "ROS2SensorsProjectGameMode.h"
#include "ROS2SensorsProjectPlayerController.h"

AROS2SensorsProjectGameMode::AROS2SensorsProjectGameMode()
{
	PlayerControllerClass = AROS2SensorsProjectPlayerController::StaticClass();
}
