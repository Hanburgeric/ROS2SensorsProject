#pragma once

#include "CoreMinimal.h"
#include "Sensors/CameraSensor.h"
#include "SemanticSegmentationCameraSensor.generated.h"

// Forward declarations
class USemanticLabelData;

DECLARE_LOG_CATEGORY_EXTERN(LogSemanticSegmentationCameraSensor, Log, All);

UCLASS( ClassGroup = (Custom), meta = (BlueprintSpawnableComponent) )
class ROS2SENSORSPROJECT_API USemanticSegmentationCameraSensor : public UCameraSensor
{
	GENERATED_BODY()
	
public:
	USemanticSegmentationCameraSensor(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

protected:
	virtual void BeginPlay() override;

protected:
	bool CanSegmentCapture() const;
	bool IsCustomDepthEnabledWithStencil() const;
	bool AreSemanticLabelsValid() const;
	bool IsBaseSegmentationMaterialValid() const;

	void InitializeSemanticSegmentation();
	UTexture2D* CreateLUTFromSemanticLabels();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<USemanticLabelData> SemanticLabels{ nullptr };

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UTexture2D> SemanticLabelLUT{ nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TObjectPtr<UMaterial> BaseSegmentationMaterial{ nullptr };

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FName LUTParameterName{ TEXT("LUT")};

	UPROPERTY(EditDefaultsOnly, BlueprintReadONly)
	FName LUTInvWidthParameterName{ TEXT("LUTInvWidth")};

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UMaterialInstanceDynamic> SegmentationMaterialInstance{ nullptr };
};
