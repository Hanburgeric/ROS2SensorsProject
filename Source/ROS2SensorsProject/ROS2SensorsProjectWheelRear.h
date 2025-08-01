// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ChaosVehicleWheel.h"
#include "ROS2SensorsProjectWheelRear.generated.h"

/**
 *  Base rear wheel definition.
 */
UCLASS()
class UROS2SensorsProjectWheelRear : public UChaosVehicleWheel
{
	GENERATED_BODY()

public:
	UROS2SensorsProjectWheelRear();
};
