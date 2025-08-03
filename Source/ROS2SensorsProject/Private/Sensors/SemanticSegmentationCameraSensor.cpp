#include "Sensors/SemanticSegmentationCameraSensor.h"

// UE
#include "CineCameraComponent.h"

// ROS2SensorsProject
#include "Sensors/SemanticLabelData.h"

DEFINE_LOG_CATEGORY(LogSemanticSegmentationCameraSensor);

USemanticSegmentationCameraSensor::USemanticSegmentationCameraSensor(const FObjectInitializer& ObjectInitializer)
	: Super{ ObjectInitializer }
{
}

void USemanticSegmentationCameraSensor::BeginPlay()
{
	Super::BeginPlay();

	if (CanSegmentCapture())
	{
		InitializeSemanticSegmentation();
	}
	else
	{
		UE_LOG(
			LogSemanticSegmentationCameraSensor, Warning,
			TEXT("Condition(s) for performing semantic segmentation")
			TEXT("has/have not been met; the component will output a regular image.")
		);
	}
}

bool USemanticSegmentationCameraSensor::CanSegmentCapture() const
{
	return IsCustomDepthEnabledWithStencil()
		&& AreSemanticLabelsValid()
		&& IsBaseSegmentationMaterialValid();
}

bool USemanticSegmentationCameraSensor::IsCustomDepthEnabledWithStencil() const
{
	if (GetCustomDepthMode() != ECustomDepthMode::EnabledWithStencil)
	{
		UE_LOG(
			LogSemanticSegmentationCameraSensor, Warning,
			TEXT("Custom depth is not enabled with stencil in project settings.")
		);

		return false;
	}

	return true;
}

bool USemanticSegmentationCameraSensor::AreSemanticLabelsValid() const
{
	if (!SemanticLabels)
	{
		UE_LOG(
			LogSemanticSegmentationCameraSensor, Warning,
			TEXT("No semantic label data asset has been provided.")
		);

		return false;
	}

	// Disallow using a data asset with no entries, as this will result in
	// attempting to create a LUT with invalid dimensions (i.e. 1x0)
	if (SemanticLabels->SemanticLabels.IsEmpty())
	{
		UE_LOG(
			LogSemanticSegmentationCameraSensor, Warning,
			TEXT("Semantic label data asset \"%s\" was provided, but contains no data."),
			*SemanticLabels->GetName()
		);

		return false;
	}

	return true;
}

bool USemanticSegmentationCameraSensor::IsBaseSegmentationMaterialValid() const
{
	if (!BaseSegmentationMaterial)
	{
		UE_LOG(
			LogSemanticSegmentationCameraSensor, Warning,
			TEXT("No base segmentation material has been provided.")
		);

		return false;
	}

	// Make sure that the material is a post process material
	if (!BaseSegmentationMaterial->IsPostProcessMaterial())
	{
		UE_LOG(
			LogSemanticSegmentationCameraSensor, Warning,
			TEXT("Base segmentation material \"%s\" was provided, ")
			TEXT("but is not a post process material."),
			*BaseSegmentationMaterial->GetName()
		);

		return false;
	}

	// Get all parameters from the material that are textures
	TMap<FMaterialParameterInfo, FMaterialParameterMetadata> MaterialTextureParameters{};
	BaseSegmentationMaterial->GetAllParametersOfType(
		EMaterialParameterType::Texture, MaterialTextureParameters
	);

	// Find the parameter with a name matching the one set in LUTParameterName
	bool bFoundLUTParameter{ false };
	for (const auto& TextureParameter : MaterialTextureParameters)
	{
		if (TextureParameter.Key.Name == LUTParameterName)
		{
			bFoundLUTParameter = true;
			break;
		}
	}

	// No matching parameter found, log and return failure
	if (!bFoundLUTParameter)
	{
		UE_LOG(
			LogSemanticSegmentationCameraSensor, Warning,
			TEXT("Base segmentation material \"%s\" was provided, ")
			TEXT("but has no texture parameter named \"%s\"."),
			*BaseSegmentationMaterial->GetName(),
			*LUTParameterName.ToString()
		);

		return false;
	}

	// Get all parameters from the material that are scalars
	TMap<FMaterialParameterInfo, FMaterialParameterMetadata> MaterialScalarParameters{};
	BaseSegmentationMaterial->GetAllParametersOfType(
		EMaterialParameterType::Scalar, MaterialScalarParameters
	);

	// Find the parameter with a name matching the one set in LUTInvWidthParameterName
	bool bFoundLUTInvWidthParameter{ false };
	for (const auto& ScalarParameter : MaterialScalarParameters)
	{
		if (ScalarParameter.Key.Name == LUTInvWidthParameterName)
		{
			bFoundLUTInvWidthParameter = true;
			break;
		}
	}

	// No matching parameter found, log and return failure
	if (!bFoundLUTInvWidthParameter)
	{
		UE_LOG(
			LogSemanticSegmentationCameraSensor, Warning,
			TEXT("Base segmentation material \"%s\" was provided, ")
			TEXT("but has no scalar parameter named \"%s\"."),
			*BaseSegmentationMaterial->GetName(),
			*LUTInvWidthParameterName.ToString()
		);

		return false;
	}

	return true;
}

void USemanticSegmentationCameraSensor::InitializeSemanticSegmentation()
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

UTexture2D* USemanticSegmentationCameraSensor::CreateLUTFromSemanticLabels()
{
	UTexture2D* LUT = NewObject<UTexture2D>(this);

	const int32 Width = SemanticLabels->SemanticLabels.Num();
	const int32 Height = 1;

	LUT->CompressionSettings = TC_VectorDisplacementmap;
	LUT->SRGB = false;
	LUT->Filter = TF_Nearest;
	LUT->NeverStream = true;
	LUT->MipGenSettings = TMGS_NoMipmaps;
	LUT->LODGroup = TEXTUREGROUP_Pixels2D;
	LUT->AddressX = TA_Clamp;
	LUT->AddressY = TA_Clamp;

	LUT->Source.Init(Width, Height, 1, 1, TSF_BGRA8);

	uint8* RawPtr = LUT->Source.LockMip(0);
	for (int32 x = 0; x < Width; ++x)
	{
		const FColor& C = SemanticLabels->SemanticLabels[x].ConvertedColor;
		RawPtr[4 * x + 0] = C.B;
		RawPtr[4 * x + 1] = C.G;
		RawPtr[4 * x + 2] = C.R;
		RawPtr[4 * x + 3] = C.A;
	}
	LUT->Source.UnlockMip(0);

	LUT->UpdateResource();

	return LUT;
}
