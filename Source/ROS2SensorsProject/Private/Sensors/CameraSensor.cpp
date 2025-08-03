#include "Sensors/CameraSensor.h"

// UE
#include "CineCameraComponent.h"
#include "CineCameraSceneCaptureComponent.h"
#include "Engine/TextureRenderTarget2D.h"

DEFINE_LOG_CATEGORY(LogCameraSensor);

UCameraSensor::UCameraSensor(const FObjectInitializer& ObjectInitializer)
	: Super{ ObjectInitializer }
	, CameraComponent{ ObjectInitializer.CreateDefaultSubobject<UCineCameraComponent>(this, TEXT("CameraComponent")) }
	, CaptureComponent{ ObjectInitializer.CreateDefaultSubobject<UCineCaptureComponent2D>(this, TEXT("CaptureComponent")) }
{
	// Attach camera to the sensor such that it inherits the sensor's transform
	CameraComponent->SetupAttachment(this);

	// A cine capture component is required to be parented to a cine camera component
	CaptureComponent->SetupAttachment(CameraComponent);
}

void UCameraSensor::BeginPlay()
{
	Super::BeginPlay();

	// If set, render the contents of the capture to a texture target
	if (CaptureComponent->TextureTarget)
	{
		// Duplicate the original texture target such that each sensor instance
		// gets its own isolated texture target and thus do not write to the same asset
		CaptureComponent->TextureTarget = DuplicateObject<UTextureRenderTarget2D>(
			CaptureComponent->TextureTarget, this
		);
	}
}
