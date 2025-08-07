#include "Sensors/CameraSensor.h"

// UE
#include "CineCameraComponent.h"
#include "CineCameraSceneCaptureComponent.h"
#include "Engine/TextureRenderTarget2D.h"

// ROS2SensorsProject
#include "Misc/SemanticLabelData.h"
#include "Utils/MaterialUtils.h"

DEFINE_LOG_CATEGORY(LogCameraSensor);

UCameraSensor::UCameraSensor()
	: CameraComponent{ CreateDefaultSubobject<UCineCameraComponent>(TEXT("CameraComponent")) }
	, CaptureComponent{ CreateDefaultSubobject<UCineCaptureComponent2D>(TEXT("CaptureComponent")) }
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

	// ???
	if (CanSegmentCapture())
	{
		InitializeSemanticSegmentationComponents();
	}
	else
	{
		UE_LOG(
			LogCameraSensor, Warning,
			TEXT("Condition(s) for performing semantic segmentation")
			TEXT("has/have not been met; the component will output a regular image.")
		);
	}
}

void UCameraSensor::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FProperty* ChangedProperty{ PropertyChangedEvent.Property };
	if (!ChangedProperty) { return; }

	const FName ChangedPropertyName{ ChangedProperty->GetFName() };
	if (ChangedPropertyName == GET_MEMBER_NAME_CHECKED(UCameraSensor, CameraType))
	{
		SetCameraType(CameraType);
	}
}

void UCameraSensor::SetCameraType(ECameraType NewCameraType)
{
	ResetCameraType();

	CameraType = NewCameraType;
	switch (CameraType)
	{
	// Do nothing; an RGB camera has no post process material to add
	case ECameraType::RGB: { break;}

	// TODO: Add depth camera post process material
	case ECameraType::Depth:
	{
		break;
	}

	// Add semantic segmentation post process material
	case ECameraType::SemanticSegmentation:
	{
		CameraComponent->AddOrUpdateBlendable(SegmentationMaterialInstance);
		break;
	}

	// This branch of logic should never be entered
	default:
		checkNoEntry();
		break;
	}
}

void UCameraSensor::ResetCameraType()
{
	// TODO: Remove depth camera post process material

	// Remove semantic segmentation post process material
	CameraComponent->RemoveBlendable(SegmentationMaterialInstance);
}

bool UCameraSensor::CanSegmentCapture() const
{
	return IsCustomDepthEnabledWithStencil()
		&& AreSemanticLabelsValid()
		&& IsBaseSegmentationMaterialValid();
}

bool UCameraSensor::IsCustomDepthEnabledWithStencil() const
{
	if (GetCustomDepthMode() != ECustomDepthMode::EnabledWithStencil)
	{
		UE_LOG(
			LogCameraSensor, Warning,
			TEXT("Custom depth is not enabled with stencil in project settings.")
		);

		return false;
	}

	return true;
}

bool UCameraSensor::AreSemanticLabelsValid() const
{
	if (!SemanticLabels)
	{
		UE_LOG(
			LogCameraSensor, Warning,
			TEXT("No semantic label data asset has been provided.")
		);

		return false;
	}

	// Disallow using a data asset with no entries, as this will result in
	// attempting to create a LUT with invalid dimensions (i.e. 1x0)
	if (SemanticLabels->SemanticLabels.IsEmpty())
	{
		UE_LOG(
			LogCameraSensor, Warning,
			TEXT("Semantic label data asset \"%s\" was provided, but contains no data."),
			*SemanticLabels->GetName()
		);

		return false;
	}

	return true;
}

bool UCameraSensor::IsBaseSegmentationMaterialValid() const
{
	if (!BaseSegmentationMaterial)
	{
		UE_LOG(
			LogCameraSensor, Warning,
			TEXT("No base segmentation material has been provided.")
		);

		return false;
	}

	// Make sure that the material is a post process material
	if (!BaseSegmentationMaterial->IsPostProcessMaterial())
	{
		UE_LOG(
			LogCameraSensor, Warning,
			TEXT("Base segmentation material \"%s\" was provided, ")
			TEXT("but is not a post process material."),
			*BaseSegmentationMaterial->GetName()
		);

		return false;
	}

	// Find a texture parameter with a name matching
	// the one set in LUTParameterName
	bool bFoundLUTParameter{
		UMaterialUtils::DoesMaterialHaveParameter(
			BaseSegmentationMaterial,
			EMaterialParameterType::Texture,
			LUTParameterName
		)
	};
	if (!bFoundLUTParameter)
	{
		UE_LOG(
			LogCameraSensor, Warning,
			TEXT("Base segmentation material \"%s\" was provided, ")
			TEXT("but has no texture parameter named \"%s\"."),
			*BaseSegmentationMaterial->GetName(),
			*LUTParameterName.ToString()
		);

		return false;
	}

	// Find a scalar parameter with a name matching
	// the one set in LUTInvWidthParameterName
	bool bFoundLUTInvWidthParameter{
		UMaterialUtils::DoesMaterialHaveParameter(
			BaseSegmentationMaterial,
			EMaterialParameterType::Scalar,
			LUTInvWidthParameterName
		)
	};
	if (!bFoundLUTInvWidthParameter)
	{
		UE_LOG(
			LogCameraSensor, Warning,
			TEXT("Base segmentation material \"%s\" was provided, ")
			TEXT("but has no scalar parameter named \"%s\"."),
			*BaseSegmentationMaterial->GetName(),
			*LUTInvWidthParameterName.ToString()
		);

		return false;
	}

	return true;
}

void UCameraSensor::InitializeSemanticSegmentationComponents()
{
	// Create a 1D LUT from the semantic labels
	SemanticLabelLUT = CreateLUTFromSemanticLabels();

	// Create an instance of the segmentation material
	// and set its texture parameter to the LUT and its scalar parameter
	// to the inverse of the LUT's width
	SegmentationMaterialInstance = UMaterialInstanceDynamic::Create(
		BaseSegmentationMaterial, this
	);
	SegmentationMaterialInstance->SetTextureParameterValue(
		LUTParameterName, SemanticLabelLUT
	);
	SegmentationMaterialInstance->SetScalarParameterValue(
		LUTInvWidthParameterName,
		1.0F / static_cast<float>(SemanticLabels->SemanticLabels.Num())
	);

	// Add the material to the camera
	CameraComponent->AddOrUpdateBlendable(SegmentationMaterialInstance);
}

UTexture2D* UCameraSensor::CreateLUTFromSemanticLabels()
{
	UTexture2D* LUT{ NewObject<UTexture2D>(this) };

	// Set LOD group as a color lookup table;
	// prevents compression and mip generation
	LUT->LODGroup = TEXTUREGROUP_ColorLookupTable;

	// Disable gamma correction, as the LUT is data,
	// not color for display; they must be in linear space
	LUT->SRGB = false;

	// Do not blend adjacent pixels, as this would mix colors
	// from different semantic labels
	LUT->Filter = TF_Nearest;

	// Initialize texture
	const int32 Width{ SemanticLabels->SemanticLabels.Num() };
	const int32 Height{ 1 };
	LUT->Source.Init(Width, Height, 1, 1, TSF_BGRA8);

	// Edit the texture
	uint8* MipData{ LUT->Source.LockMip(0) };
	for (int32 x{ 0 }; x < Width; ++x)
	{
		// Get the color corresponding to the semantic label
		// at the current texture index
		const FColor& PixelColor{
			SemanticLabels->SemanticLabels[x].ConvertedColor
		};

		// Write the color to the texture data (4 bytes per pixel)
		const int32 MipDataIndex{ x * 4 };
		MipData[MipDataIndex + 0] = PixelColor.B;
		MipData[MipDataIndex + 1] = PixelColor.G;
		MipData[MipDataIndex + 2] = PixelColor.R;
		MipData[MipDataIndex + 3] = PixelColor.A;
	}
	LUT->Source.UnlockMip(0);

	// Update the texture
	LUT->UpdateResource();

	return LUT;
}
