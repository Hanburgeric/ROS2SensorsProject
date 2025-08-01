// Copyright Epic Games, Inc. All Rights Reserved.

#include "ROS2SensorsProjectWheelFront.h"
#include "UObject/ConstructorHelpers.h"

UROS2SensorsProjectWheelFront::UROS2SensorsProjectWheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 40.f;
}