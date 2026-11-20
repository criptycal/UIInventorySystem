#include "UI/InventoryWidget.h"

#include "UI/Preview/InventoryCharacterPreview.h"
#include "Components/Image.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Components/Border.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"
#include "Inventory/Components/EquipmentComponent.h"
#include "Inventory/Components/InventoryComponent.h"
#include "Inventory/Data/ItemRow.h"

void UInventoryWidget::CreateCharacterPreview()
{
	ReleaseCharacterPreview();

	ACharacter* PlayerCharacter = Cast<ACharacter>(GetOwningPlayerPawn());

	UWorld* World = GetWorld();

	if (!CharacterPreviewImage || !PlayerCharacter || !World) return;

	CharacterPreviewMaterial = CharacterPreviewImage->GetDynamicMaterial();

	if (!CharacterPreviewMaterial)
	{
		UE_LOG(LogTemp, Warning, TEXT("Character preview: the Image needs a material brush."));
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.ObjectFlags |= RF_Transient;
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	const FVector PreviewLocation = PlayerCharacter->GetActorLocation() + FVector(0.f, 0.f, 10000.f);

	CharacterPreviewActor =
		World->SpawnActor<AInventoryCharacterPreview>(
			PreviewLocation,
			FRotator::ZeroRotator,
			SpawnParameters
		);

	if (!IsValid(CharacterPreviewActor.Get()))
	{
		ReleaseCharacterPreview();
		return;
	}

	if (!CharacterPreviewActor->InitializeFromMesh(PlayerCharacter->GetMesh(), CharacterPreviewIdleAnimation.Get()))
	{
		ReleaseCharacterPreview();
		return;
	}

	CharacterPreviewActor->SetPresentationSettings(
		PreviewCameraDistance, PreviewCameraHeight,
		PreviewKeyLightIntensity, PreviewFillLightIntensity);

	CharacterPreviewMaterial->SetTextureParameterValue(
		TEXT("PreviewTexture"),
		CharacterPreviewActor->GetRenderTarget());

	ObservedEquipment = PlayerCharacter->FindComponentByClass<UEquipmentComponent>();

	if (ObservedEquipment.IsValid())
	{
		ObservedEquipment->OnEquipmentChanged.AddUObject(this, &UInventoryWidget::RefreshPreviewEquipment);
	}
	RefreshPreviewEquipment();
	CharacterPreviewImage->SetVisibility(ESlateVisibility::HitTestInvisible);
}

void UInventoryWidget::ReleaseCharacterPreview()
{
	if (ObservedEquipment.IsValid())
	{
		ObservedEquipment->OnEquipmentChanged.RemoveAll(this);
	}

	ObservedEquipment.Reset();

	bDraggingCharacterPreview = false;
	if (CharacterPreviewImage)
	{
		CharacterPreviewImage->SetVisibility(ESlateVisibility::Hidden);
	}

	if (CharacterPreviewMaterial)
	{
		CharacterPreviewMaterial->SetTextureParameterValue(TEXT("PreviewTexture"), nullptr);
		CharacterPreviewMaterial = nullptr;
	}

	if (IsValid(CharacterPreviewActor.Get()))
	{
		CharacterPreviewActor->Destroy();
	}

	CharacterPreviewActor = nullptr;
}

FReply UInventoryWidget::NativeOnMouseButtonDown(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	SetKeyboardFocus();

	const FVector2D MousePosition =
		InMouseEvent.GetScreenSpacePosition();

	const bool bOverPreview =
		CharacterPreviewImage &&
		CharacterPreviewImage->IsVisible() &&
		CharacterPreviewImage->GetCachedGeometry()
		.IsUnderLocation(MousePosition);

	const bool bOverDescription =
		ItemInfoPanel &&
		ItemInfoPanel->IsVisible() &&
		ItemInfoPanel->GetCachedGeometry()
		.IsUnderLocation(MousePosition);

	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton &&
		bOverPreview &&
		!bOverDescription &&
		!ActionMenu &&
		IsValid(CharacterPreviewActor.Get()))
	{
		bDraggingCharacterPreview = true;

		return FReply::Handled().CaptureMouse(TakeWidget());
	}

	return FReply::Handled();
}


FReply UInventoryWidget::NativeOnMouseMove(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (!bDraggingCharacterPreview)
	{
		return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
	}

	if (!InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) ||
		!IsValid(CharacterPreviewActor.Get()) ||
		ActionMenu)
	{
		bDraggingCharacterPreview = false;

		return FReply::Handled().ReleaseMouseCapture();
	}

	const FVector2D CurrentPosition = InGeometry.AbsoluteToLocal(
		InMouseEvent.GetScreenSpacePosition());

	const FVector2D PreviousPosition = InGeometry.AbsoluteToLocal(
		InMouseEvent.GetLastScreenSpacePosition());

	const float HorizontalMovement =
		static_cast<float>(CurrentPosition.X - PreviousPosition.X);

	CharacterPreviewActor->RotatePreview(
		-HorizontalMovement * PreviewRotationSensitivity);

	return FReply::Handled();
}


FReply UInventoryWidget::NativeOnMouseButtonUp(
	const FGeometry& InGeometry,
	const FPointerEvent& InMouseEvent)
{
	if (bDraggingCharacterPreview &&
		InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bDraggingCharacterPreview = false;

		return FReply::Handled().ReleaseMouseCapture();
	}

	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UInventoryWidget::NativeOnMouseCaptureLost(
	const FCaptureLostEvent& CaptureLostEvent)
{
	bDraggingCharacterPreview = false;

	Super::NativeOnMouseCaptureLost(CaptureLostEvent);
}

UStaticMesh* UInventoryWidget::FindPreviewEquipmentMesh(const FGuid& EntryId) const
{
	if (!ObservedInventory.IsValid() || !EntryId.IsValid()) return nullptr;

	const FInventoryEntry* Entry = ObservedInventory->FindEntry(EntryId);

	if (!Entry) return nullptr;

	const FItemRow* Definition = ObservedInventory->FindItemDefinition(Entry->ItemRowName);

	return Definition ? Definition->WorldMesh.Get() : nullptr;
}

void UInventoryWidget::RefreshPreviewEquipment()
{
	if (!IsValid(CharacterPreviewActor.Get())) return;

	if (!ObservedEquipment.IsValid())
	{
		CharacterPreviewActor->SetEquipmentMeshes(nullptr, nullptr);
		return;
	}

	UStaticMesh* WeaponMesh = FindPreviewEquipmentMesh(ObservedEquipment->GetEquippedWeaponId());
	UStaticMesh* ShieldMesh = FindPreviewEquipmentMesh(ObservedEquipment->GetEquippedShieldId());

	CharacterPreviewActor->SetEquipmentMeshes(WeaponMesh, ShieldMesh);



}
