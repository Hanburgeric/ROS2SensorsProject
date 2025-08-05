#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CameraSensor.generated.h"

// Forward declarations
class UCineCameraComponent;
class UCineCaptureComponent2D;

DECLARE_LOG_CATEGORY_EXTERN(LogCameraSensor, Log, All);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROS2SENSORSPROJECT_API UCameraSensor : public USceneComponent
{
	GENERATED_BODY()

public:
	UCameraSensor();

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UCineCameraComponent> CameraComponent{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintREadOnly)
	TObjectPtr<UCineCaptureComponent2D> CaptureComponent{ nullptr };
};
