// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ROS2SensorsProjectPawn.h"
#include "ROS2SensorsProjectSportsCar.generated.h"

/**
 *  Sports car wheeled vehicle implementation
 */
UCLASS(abstract)
class ROS2SENSORSPROJECT_API AROS2SensorsProjectSportsCar : public AROS2SensorsProjectPawn
{
	GENERATED_BODY()
	
public:

	AROS2SensorsProjectSportsCar();
};
