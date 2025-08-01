// Copyright Epic Games, Inc. All Rights Reserved.

#include "ROS2SensorsProjectWheelRear.h"
#include "UObject/ConstructorHelpers.h"

UROS2SensorsProjectWheelRear::UROS2SensorsProjectWheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
}