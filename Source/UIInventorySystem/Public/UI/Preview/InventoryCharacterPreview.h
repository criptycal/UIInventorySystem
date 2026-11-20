// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InventoryCharacterPreview.generated.h"

class USceneComponent;
class USkeletalMeshComponent;
class USceneCaptureComponent2D;
class UTextureRenderTarget2D;
class UAnimSequence;
class UPointLightComponent;
class UStaticMeshComponent;
class UStaticMesh;


UCLASS()
class UIINVENTORYSYSTEM_API AInventoryCharacterPreview : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AInventoryCharacterPreview();

	bool InitializeFromMesh(const USkeletalMeshComponent* SourceMesh, UAnimSequence* IdleAnimation);

	void SetPresentationSettings(float CameraDistance, float CameraHeight,
		float KeyLightIntensity, float FillLightIntensity);

	UTextureRenderTarget2D* GetRenderTarget() const { return RenderTarget.Get(); }

	void RotatePreview(float DeltaYaw);

	void SetEquipmentMeshes(UStaticMesh* WeaponMesh, UStaticMesh* ShieldMesh);

private:
	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<USceneComponent> PreviewRoot;

	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<USkeletalMeshComponent> CharacterMesh;

	UPROPERTY(VisibleAnywhere, Category = "Preview|Lighting")
	TObjectPtr<UPointLightComponent> KeyLight;

	UPROPERTY(VisibleAnywhere, Category = "Preview|Lighting")
	TObjectPtr<UPointLightComponent> FillLight;

	UPROPERTY(VisibleAnywhere, Category = "Preview")
	TObjectPtr<USceneCaptureComponent2D> SceneCapture;

	UPROPERTY(Transient)
	TObjectPtr<UTextureRenderTarget2D> RenderTarget = nullptr;

	void CreateRenderTarget();

	void ConfigureCapture();

	UPROPERTY(VisibleAnywhere, Category = "Preview|Equipment")
	TObjectPtr<UStaticMeshComponent> PreviewWeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Preview|Equipment")
	TObjectPtr<UStaticMeshComponent> PreviewShieldMesh;


};
