// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/Preview/InventoryCharacterPreview.h"

#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/PointLightComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Animation/AnimSequence.h"
#include "Components/StaticMeshComponent.h"


// Sets default values
AInventoryCharacterPreview::AInventoryCharacterPreview()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	PreviewRoot = CreateDefaultSubobject<USceneComponent>(TEXT("PreviewRoot"));
	SetRootComponent(PreviewRoot);

	CharacterMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh"));

	CharacterMesh->SetupAttachment(PreviewRoot);
	CharacterMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CharacterMesh->SetGenerateOverlapEvents(false);

	CharacterMesh->SetVisibleInSceneCaptureOnly(true);
	CharacterMesh->SetCastShadow(false);

	CharacterMesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::AlwaysTickPoseAndRefreshBones;

	// Reserve channel 1 for the preview's direct lighting.
	CharacterMesh->SetLightingChannels(false, true, false);

	// Keep lights on the root so rotating the character won't rotate its lighting.
	KeyLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("KeyLight"));
	KeyLight->SetupAttachment(PreviewRoot);
	KeyLight->SetMobility(EComponentMobility::Movable);
	KeyLight->SetRelativeLocation(FVector(200.f, -150.f, 220.f));
	KeyLight->SetIntensityUnits(ELightUnits::Lumens);
	KeyLight->SetIntensity(500.f);
	KeyLight->SetAttenuationRadius(600.f);
	KeyLight->SetLightingChannels(false, true, false);

	PreviewWeaponMesh =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewWeaponMesh"));

	PreviewWeaponMesh->SetupAttachment(CharacterMesh, TEXT("WeaponSocket"));
	PreviewWeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewWeaponMesh->SetGenerateOverlapEvents(false);
	PreviewWeaponMesh->SetCanEverAffectNavigation(false);
	PreviewWeaponMesh->SetVisibleInSceneCaptureOnly(true);
	PreviewWeaponMesh->SetLightingChannels(false, true, false);
	PreviewWeaponMesh->SetCastShadow(false);

	PreviewShieldMesh =
		CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PreviewShieldMesh"));

	PreviewShieldMesh->SetupAttachment(CharacterMesh, TEXT("ShieldSocket"));
	PreviewShieldMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PreviewShieldMesh->SetGenerateOverlapEvents(false);
	PreviewShieldMesh->SetCanEverAffectNavigation(false);
	PreviewShieldMesh->SetVisibleInSceneCaptureOnly(true);
	PreviewShieldMesh->SetLightingChannels(false, true, false);
	PreviewShieldMesh->SetCastShadow(false);

	KeyLight->SetCastShadows(false);

	FillLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("FillLight"));
	FillLight->SetupAttachment(PreviewRoot);
	FillLight->SetMobility(EComponentMobility::Movable);
	FillLight->SetRelativeLocation(FVector(150.f, 180.f, 120.f));
	FillLight->SetIntensityUnits(ELightUnits::Lumens);
	FillLight->SetIntensity(150.f);
	FillLight->SetAttenuationRadius(600.f);
	FillLight->SetLightingChannels(false, true, false);
	FillLight->SetCastShadows(false);

	SceneCapture = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));

	SceneCapture->SetupAttachment(PreviewRoot);
	SceneCapture->bCaptureEveryFrame = true;
	SceneCapture->bCaptureOnMovement = false;

	SceneCapture->CaptureSource = SCS_SceneColorHDR;

}

bool AInventoryCharacterPreview::InitializeFromMesh(const USkeletalMeshComponent* SourceMesh,
	UAnimSequence* IdleAnimation)
{
	if(!IsValid(SourceMesh)) return false;

	if (!IsValid(IdleAnimation))
	{
		UE_LOG(LogTemp, Warning, TEXT("Character preview: Idle animation is not valid."));
		return false;
	}

	USkeletalMesh* MeshAsset = SourceMesh->GetSkeletalMeshAsset();

	if (!MeshAsset) return false; // If the source mesh doesn't have a valid skeletal mesh asset, we cannot initialize the preview.

	CreateRenderTarget(); // Ensure the render target is created before setting it to the scene capture component.

	CharacterMesh->SetSkeletalMesh(MeshAsset);

	CharacterMesh->SetRelativeRotation(SourceMesh->GetRelativeRotation());

	CharacterMesh->EmptyOverrideMaterials();

	const int32 MaterialCount = SourceMesh->GetNumMaterials();

	for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
	{
		CharacterMesh->SetMaterial(MaterialIndex, SourceMesh->GetMaterial(MaterialIndex));
	}

	ConfigureCapture(); // Configure the scene capture settings after setting up the mesh.
	CharacterMesh->PlayAnimation(IdleAnimation, true); // Play the idle animation in a loop.
	return true;
}

void AInventoryCharacterPreview::SetPresentationSettings(float CameraDistance, float CameraHeight,
	float KeyLightIntensity, float FillLightIntensity)
{
	SceneCapture->SetRelativeLocation(FVector(FMath::Max(50.f, CameraDistance), 0.f, CameraHeight));
	KeyLight->SetIntensity(FMath::Max(0.f, KeyLightIntensity));
	FillLight->SetIntensity(FMath::Max(0.f, FillLightIntensity));
}

void AInventoryCharacterPreview::CreateRenderTarget()
{
	if (RenderTarget) return;

	// Create a new transient render target for the scene capture
	RenderTarget = NewObject<UTextureRenderTarget2D>(this, NAME_None, RF_Transient);

	RenderTarget->RenderTargetFormat = RTF_RGBA16f;
	RenderTarget->ClearColor = FLinearColor(0.f, 0.f, 0.f, 1.f);
	RenderTarget->bAutoGenerateMips = false;
	RenderTarget->InitAutoFormat(1024, 1024); // Leave room for equipment on both sides.

	SceneCapture->TextureTarget = RenderTarget;
}

void AInventoryCharacterPreview::ConfigureCapture()
{
	SceneCapture->PrimitiveRenderMode =
		ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;

	SceneCapture->ClearShowOnlyComponents();
	SceneCapture->ShowOnlyComponent(CharacterMesh);

	SceneCapture->ShowOnlyComponent(PreviewWeaponMesh);
	SceneCapture->ShowOnlyComponent(PreviewShieldMesh);

	SceneCapture->SetRelativeLocation(FVector(280.f, 0.f, 90.f));
	SceneCapture->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	// Preserve the previous vertical framing with the wider, square render target.
	SceneCapture->FOVAngle = 72.1f;

	SceneCapture->ShowFlags.SetAtmosphere(false);
	SceneCapture->ShowFlags.SetFog(false);
}

void AInventoryCharacterPreview::RotatePreview(float DeltaYaw)
{
	if (!CharacterMesh || !FMath::IsFinite(DeltaYaw)) return;
	FRotator Rotation = CharacterMesh->GetRelativeRotation();
	Rotation.Yaw = FRotator::NormalizeAxis(Rotation.Yaw + DeltaYaw);
	CharacterMesh->SetRelativeRotation(Rotation);
}

void AInventoryCharacterPreview::SetEquipmentMeshes(UStaticMesh* WeaponMesh, UStaticMesh* ShieldMesh)
{
	if (PreviewWeaponMesh)
	{
		PreviewWeaponMesh->SetStaticMesh(WeaponMesh);
	}
	if (PreviewShieldMesh)
	{
		PreviewShieldMesh->SetStaticMesh(ShieldMesh);
	}
}
