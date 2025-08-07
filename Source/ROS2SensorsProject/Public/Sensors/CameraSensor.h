#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "CameraSensor.generated.h"

// Forward declarations
class UCineCameraComponent;
class UCineCaptureComponent2D;
class USemanticLabelData;

DECLARE_LOG_CATEGORY_EXTERN(LogCameraSensor, Log, All);

UENUM(BlueprintType)
enum class ECameraType : uint8
{
	RGB						UMETA(DisplayName = "RGB"),
	Depth					UMETA(DisplayName = "Depth"),
	SemanticSegmentation	UMETA(DisplayName = "Semantic Segmentation")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ROS2SENSORSPROJECT_API UCameraSensor : public USceneComponent
{
	GENERATED_BODY()

public:
	UCameraSensor();

protected:
	virtual void BeginPlay() override;

protected:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

public:
	UFUNCTION(BlueprintCallable)
	void SetCameraType(ECameraType NewCameraType);

protected:
	void ResetCameraType();

	bool CanSegmentCapture() const;
	bool IsCustomDepthEnabledWithStencil() const;
	bool AreSemanticLabelsValid() const;
	bool IsBaseSegmentationMaterialValid() const;

	void InitializeSemanticSegmentationComponents();
	UTexture2D* CreateLUTFromSemanticLabels();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter = "SetCameraType", Category = "Camera Sensor")
	ECameraType CameraType{ ECameraType::RGB };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera Sensor")
	TObjectPtr<UCineCameraComponent> CameraComponent{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera Sensor")
	TObjectPtr<UCineCaptureComponent2D> CaptureComponent{ nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Semantic Segmentation")
	TObjectPtr<USemanticLabelData> SemanticLabels{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Semantic Segmentation")
	TObjectPtr<UTexture2D> SemanticLabelLUT{ nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Semantic Segmentation")
	TObjectPtr<UMaterial> BaseSegmentationMaterial{ nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Semantic Segmentation")
	FName LUTParameterName{ TEXT("LUT") };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Semantic Segmentation")
	FName LUTInvWidthParameterName{ TEXT("LUTInvWidth") };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Semantic Segmentation")
	TObjectPtr<UMaterialInstanceDynamic> SegmentationMaterialInstance{ nullptr };
};
