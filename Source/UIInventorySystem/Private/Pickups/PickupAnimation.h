#pragma once

#include "CoreMinimal.h"

namespace PickupAnimation
{
    // Preserve the authored pose, but spin about world up through the geometry center.
    inline FTransform MakeTransform(const FTransform& RestWorldTransform,
        const FVector& LocalMeshCenter, float SpinDegrees, float HeightOffset)
    {
        const FQuat Rotation = FQuat(FVector::UpVector, FMath::DegreesToRadians(SpinDegrees))
            * RestWorldTransform.GetRotation();
        const FVector Center = RestWorldTransform.TransformPosition(LocalMeshCenter)
            + FVector::UpVector * HeightOffset;
        const FVector PivotOffset = Rotation.RotateVector(
            RestWorldTransform.GetScale3D() * LocalMeshCenter);
        return FTransform(Rotation, Center - PivotOffset, RestWorldTransform.GetScale3D());
    }
}
