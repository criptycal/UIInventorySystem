#include "Pickups/PickupAnimation.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FPickupAnimationTest,
    "UIInventorySystem.Pickups.WorldUpAnimation",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FPickupAnimationTest::RunTest(const FString& Parameters)
{
    const FTransform Rest(FRotator(65., 30., -40.), FVector(100., -50., 90.), FVector(.35, .6, 1.2));
    const FVector MeshCenter(40., -20., 10.);
    const FVector WorldCenter = Rest.TransformPosition(MeshCenter);
    for (const float Angle : {0.f, 90.f, 180.f, 270.f, 360.f})
    {
        const FTransform Animated = PickupAnimation::MakeTransform(Rest, MeshCenter, Angle, 15.f);
        TestTrue(TEXT("Off-center geometry bobs vertically without orbiting"),
            Animated.TransformPosition(MeshCenter).Equals(WorldCenter + FVector(0., 0., 15.), .001));
        TestTrue(TEXT("Authored scale is preserved"), Animated.GetScale3D().Equals(Rest.GetScale3D()));
        const FVector OriginalAxis = Rest.GetRotation().RotateVector(FVector::UpVector);
        const FVector AnimatedAxis = Animated.GetRotation().RotateVector(FVector::UpVector);
        TestTrue(TEXT("World-up rotation preserves the mesh's tilt"),
            FMath::IsNearlyEqual(OriginalAxis.Z, AnimatedAxis.Z, .001));
    }
    TestTrue(TEXT("Animation starts at the authored pose"),
        PickupAnimation::MakeTransform(Rest, MeshCenter, 0.f, 0.f).Equals(Rest, .001));
    TestTrue(TEXT("A full turn returns to the authored pose"),
        PickupAnimation::MakeTransform(Rest, MeshCenter, 360.f, 0.f).Equals(Rest, .001));
    return true;
}
#endif
