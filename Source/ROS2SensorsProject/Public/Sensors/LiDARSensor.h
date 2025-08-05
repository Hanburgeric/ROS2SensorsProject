#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "LiDARSensor.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROS2SENSORSPROJECT_API ULiDARSensor : public USceneComponent
{
	GENERATED_BODY()

public:
	ULiDARSensor();

protected:
	virtual void BeginPlay() override;

};
