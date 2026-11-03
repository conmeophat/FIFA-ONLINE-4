#include "FootballStadiumActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "UObject/ConstructorHelpers.h"

namespace FootballStadium
{
    constexpr float CubeSizeCm = 100.0f;
    constexpr float LineWidthCm = 10.0f;
    constexpr float LineHeightCm = 3.0f;
    constexpr float GoalWidthCm = 732.0f;
    constexpr float GoalHeightCm = 244.0f;
    constexpr float GoalDepthCm = 220.0f;
}

AFootballStadiumActor::AFootballStadiumActor()
{
    PrimaryActorTick.bCanEverTick = false;

    SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
    SetRootComponent(SceneRoot);

    PitchSurface = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PitchSurface"));
    PitchSurface->SetupAttachment(SceneRoot);
    PitchSurface->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(
        TEXT("/Engine/BasicShapes/Cube.Cube")
    );

    if (CubeAsset.Succeeded())
    {
        UnitCubeMesh = CubeAsset.Object;
        PitchSurface->SetStaticMesh(UnitCubeMesh);
    }
}

void AFootballStadiumActor::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);

    if (!UnitCubeMesh)
    {
        return;
    }

    ClearGeneratedComponents();

    const float HalfLength = PitchLengthCm * 0.5f;
    const float HalfWidth = PitchWidthCm * 0.5f;

    // The default Engine cube is 100 cm on each axis. Its top surface sits at Z=0.
    PitchSurface->SetRelativeLocation(FVector(0.0f, 0.0f, -10.0f));
    PitchSurface->SetRelativeScale3D(FVector(
        PitchLengthCm / FootballStadium::CubeSizeCm,
        PitchWidthCm / FootballStadium::CubeSizeCm,
        0.20f
    ));
    if (PitchMaterial)
    {
        PitchSurface->SetMaterial(0, PitchMaterial);
    }

    BuildPitchMarkings();
    BuildPenaltyAreas();
    BuildGoals();
    BuildCornerFlags();

    if (bBuildStands)
    {
        BuildStands();
    }

    if (bBuildFloodlightTowers)
    {
        BuildFloodlightTowers();
    }

    // Keep the dimensions visible in the actor's component layout for quick inspection.
    (void)HalfLength;
    (void)HalfWidth;
}

void AFootballStadiumActor::ClearGeneratedComponents()
{
    for (UStaticMeshComponent* Component : GeneratedComponents)
    {
        if (IsValid(Component))
        {
            Component->DestroyComponent();
        }
    }

    GeneratedComponents.Reset();
}

UStaticMeshComponent* AFootballStadiumActor::AddBox(
    const FName ComponentName,
    const FVector& CenterCm,
    const FVector& SizeCm,
    const FRotator& Rotation,
    UMaterialInterface* Material,
    const bool bEnableCollision
)
{
    if (!UnitCubeMesh)
    {
        return nullptr;
    }

    UStaticMeshComponent* Component = NewObject<UStaticMeshComponent>(this, ComponentName);
    if (!Component)
    {
        return nullptr;
    }

    Component->SetupAttachment(SceneRoot);
    AddInstanceComponent(Component);
    Component->SetStaticMesh(UnitCubeMesh);
    Component->SetRelativeLocation(CenterCm);
    Component->SetRelativeRotation(Rotation);
    Component->SetRelativeScale3D(FVector(
        FMath::Max(0.01f, SizeCm.X / FootballStadium::CubeSizeCm),
        FMath::Max(0.01f, SizeCm.Y / FootballStadium::CubeSizeCm),
        FMath::Max(0.01f, SizeCm.Z / FootballStadium::CubeSizeCm)
    ));

    if (Material)
    {
        Component->SetMaterial(0, Material);
    }

    Component->SetCollisionEnabled(
        bEnableCollision ? ECollisionEnabled::QueryAndPhysics : ECollisionEnabled::NoCollision
    );
    Component->SetGenerateOverlapEvents(false);
    Component->RegisterComponent();

    GeneratedComponents.Add(Component);
    return Component;
}

void AFootballStadiumActor::BuildPitchMarkings()
{
    const float HalfLength = PitchLengthCm * 0.5f;
    const float HalfWidth = PitchWidthCm * 0.5f;
    const FVector LineSizeZ(0.0f, 0.0f, FootballStadium::LineHeightCm);

    auto AddMarking = [this, &LineSizeZ](const FName Name, const FVector& Center,
                                       const FVector2D& Size, const FRotator& Rotation = FRotator())
    {
        AddBox(
            Name,
            FVector(Center.X, Center.Y, 2.0f),
            FVector(Size.X, Size.Y, LineSizeZ.Z),
            Rotation,
            MarkingMaterial
        );
    };

    // Outer touchlines and goal lines.
    AddMarking(TEXT("Touchline_North"), FVector(0, HalfWidth, 0), FVector2D(PitchLengthCm, FootballStadium::LineWidthCm));
    AddMarking(TEXT("Touchline_South"), FVector(0, -HalfWidth, 0), FVector2D(PitchLengthCm, FootballStadium::LineWidthCm));
    AddMarking(TEXT("GoalLine_Left"), FVector(-HalfLength, 0, 0), FVector2D(FootballStadium::LineWidthCm, PitchWidthCm));
    AddMarking(TEXT("GoalLine_Right"), FVector(HalfLength, 0, 0), FVector2D(FootballStadium::LineWidthCm, PitchWidthCm));

    // Halfway line and centre circle.
    AddMarking(TEXT("HalfwayLine"), FVector(0, 0, 0), FVector2D(FootballStadium::LineWidthCm, PitchWidthCm));
    BuildCircle(TEXT("CenterCircle"), FVector2D(0.0f, 0.0f), 915.0f, 48);

    AddBox(
        TEXT("CenterSpot"),
        FVector(0, 0, 2),
        FVector(28, 28, FootballStadium::LineHeightCm),
        FRotator::ZeroRotator,
        MarkingMaterial
    );
}

void AFootballStadiumActor::BuildPenaltyAreas()
{
    const float HalfLength = PitchLengthCm * 0.5f;
    const float HalfPenaltyWidth = PenaltyAreaWidthCm * 0.5f;
    const float HalfWidth = PitchWidthCm * 0.5f;
    const float GoalAreaDepthCm = 550.0f;
    const float GoalAreaWidthCm = 1832.0f;
    const float HalfGoalAreaWidth = GoalAreaWidthCm * 0.5f;

    for (const int32 Side : { -1, 1 })
    {
        const float GoalX = static_cast<float>(Side) * HalfLength;
        const float PenaltyInnerX = GoalX - static_cast<float>(Side) * PenaltyAreaDepthCm;
        const float PenaltyMidX = (GoalX + PenaltyInnerX) * 0.5f;

        AddBox(
            FName(*FString::Printf(TEXT("PenaltyBox_%d_Top"), Side)),
            FVector(PenaltyMidX, HalfPenaltyWidth, 2),
            FVector(PenaltyAreaDepthCm, FootballStadium::LineWidthCm, FootballStadium::LineHeightCm),
            FRotator::ZeroRotator,
            MarkingMaterial
        );
        AddBox(
            FName(*FString::Printf(TEXT("PenaltyBox_%d_Bottom"), Side)),
            FVector(PenaltyMidX, -HalfPenaltyWidth, 2),
            FVector(PenaltyAreaDepthCm, FootballStadium::LineWidthCm, FootballStadium::LineHeightCm),
            FRotator::ZeroRotator,
            MarkingMaterial
        );
        AddBox(
            FName(*FString::Printf(TEXT("PenaltyBox_%d_Inner"), Side)),
            FVector(PenaltyInnerX, 0, 2),
            FVector(FootballStadium::LineWidthCm, PenaltyAreaWidthCm, FootballStadium::LineHeightCm),
            FRotator::ZeroRotator,
            MarkingMaterial
        );

        const float GoalAreaInnerX = GoalX - static_cast<float>(Side) * GoalAreaDepthCm;
        const float GoalAreaMidX = (GoalX + GoalAreaInnerX) * 0.5f;

        AddBox(
            FName(*FString::Printf(TEXT("GoalArea_%d_Top"), Side)),
            FVector(GoalAreaMidX, HalfGoalAreaWidth, 2),
            FVector(GoalAreaDepthCm, FootballStadium::LineWidthCm, FootballStadium::LineHeightCm),
            FRotator::ZeroRotator,
            MarkingMaterial
        );
        AddBox(
            FName(*FString::Printf(TEXT("GoalArea_%d_Bottom"), Side)),
            FVector(GoalAreaMidX, -HalfGoalAreaWidth, 2),
            FVector(GoalAreaDepthCm, FootballStadium::LineWidthCm, FootballStadium::LineHeightCm),
            FRotator::ZeroRotator,
            MarkingMaterial
        );
        AddBox(
            FName(*FString::Printf(TEXT("GoalArea_%d_Inner"), Side)),
            FVector(GoalAreaInnerX, 0, 2),
            FVector(FootballStadium::LineWidthCm, GoalAreaWidthCm, FootballStadium::LineHeightCm),
            FRotator::ZeroRotator,
            MarkingMaterial
        );

        // Penalty mark: small white square blockout.
        AddBox(
            FName(*FString::Printf(TEXT("PenaltySpot_%d"), Side)),
            FVector(GoalX - static_cast<float>(Side) * 1100.0f, 0, 2),
            FVector(24, 24, FootballStadium::LineHeightCm),
            FRotator::ZeroRotator,
            MarkingMaterial
        );
    }

    (void)HalfWidth;
}

void AFootballStadiumActor::BuildGoals()
{
    const float HalfLength = PitchLengthCm * 0.5f;
    const float HalfGoalWidth = FootballStadium::GoalWidthCm * 0.5f;
    const float PostThicknessCm = 12.0f;

    for (const int32 Side : { -1, 1 })
    {
        const float FrontX = static_cast<float>(Side) * (HalfLength + 10.0f);
        const float BackX = static_cast<float>(Side) * (HalfLength + FootballStadium::GoalDepthCm);
        const float GoalCenterX = (FrontX + BackX) * 0.5f;

        for (const int32 PostSide : { -1, 1 })
        {
            const float Y = static_cast<float>(PostSide) * HalfGoalWidth;

            AddBox(
                FName(*FString::Printf(TEXT("Goal_%d_FrontPost_%d"), Side, PostSide)),
                FVector(FrontX, Y, FootballStadium::GoalHeightCm * 0.5f),
                FVector(PostThicknessCm, PostThicknessCm, FootballStadium::GoalHeightCm),
                FRotator::ZeroRotator,
                GoalMaterial,
                true
            );

            AddBox(
                FName(*FString::Printf(TEXT("Goal_%d_BackPost_%d"), Side, PostSide)),
                FVector(BackX, Y, FootballStadium::GoalHeightCm * 0.5f),
                FVector(PostThicknessCm, PostThicknessCm, FootballStadium::GoalHeightCm),
                FRotator::ZeroRotator,
                GoalMaterial,
                true
            );

            AddBox(
                FName(*FString::Printf(TEXT("Goal_%d_SideRail_%d"), Side, PostSide)),
                FVector(GoalCenterX, Y, FootballStadium::GoalHeightCm),
                FVector(FootballStadium::GoalDepthCm, PostThicknessCm, PostThicknessCm),
                FRotator::ZeroRotator,
                GoalMaterial,
                true
            );
        }

        AddBox(
            FName(*FString::Printf(TEXT("Goal_%d_Crossbar_Front"), Side)),
            FVector(FrontX, 0, FootballStadium::GoalHeightCm),
            FVector(PostThicknessCm, FootballStadium::GoalWidthCm, PostThicknessCm),
            FRotator::ZeroRotator,
            GoalMaterial,
            true
        );

        AddBox(
            FName(*FString::Printf(TEXT("Goal_%d_Crossbar_Back"), Side)),
            FVector(BackX, 0, FootballStadium::GoalHeightCm),
            FVector(PostThicknessCm, FootballStadium::GoalWidthCm, PostThicknessCm),
            FRotator::ZeroRotator,
            GoalMaterial,
            true
        );
    }
}

void AFootballStadiumActor::BuildCornerFlags()
{
    const float HalfLength = PitchLengthCm * 0.5f;
    const float HalfWidth = PitchWidthCm * 0.5f;

    for (const int32 XSide : { -1, 1 })
    {
        for (const int32 YSide : { -1, 1 })
        {
            AddBox(
                FName(*FString::Printf(TEXT("CornerFlag_%d_%d"), XSide, YSide)),
                FVector(
                    static_cast<float>(XSide) * HalfLength,
                    static_cast<float>(YSide) * HalfWidth,
                    75.0f
                ),
                FVector(6.0f, 6.0f, 150.0f),
                FRotator::ZeroRotator,
                GoalMaterial
            );
        }
    }
}

void AFootballStadiumActor::BuildStands()
{
    const float HalfLength = PitchLengthCm * 0.5f;
    const float HalfWidth = PitchWidthCm * 0.5f;

    // Simple three-tier blockout. Replace with licensed / original modular seat meshes later.
    for (const int32 Side : { -1, 1 })
    {
        for (int32 Tier = 0; Tier < 3; ++Tier)
        {
            const float OutwardOffset = HalfWidth + 500.0f + Tier * 850.0f;
            const float Height = 300.0f + Tier * 260.0f;
            const float Depth = 900.0f;

            AddBox(
                FName(*FString::Printf(TEXT("SidelineStand_%d_Tier_%d"), Side, Tier)),
                FVector(0, static_cast<float>(Side) * OutwardOffset, Height * 0.5f),
                FVector(PitchLengthCm + 2600.0f, Depth, Height),
                FRotator::ZeroRotator,
                StructureMaterial
            );

            const float EndOffset = HalfLength + 600.0f + Tier * 850.0f;
            AddBox(
                FName(*FString::Printf(TEXT("GoalEndStand_%d_Tier_%d"), Side, Tier)),
                FVector(static_cast<float>(Side) * EndOffset, 0, Height * 0.5f),
                FVector(Depth, PitchWidthCm + 2200.0f, Height),
                FRotator::ZeroRotator,
                StructureMaterial
            );
        }
    }
}

void AFootballStadiumActor::BuildFloodlightTowers()
{
    const float HalfLength = PitchLengthCm * 0.5f;
    const float HalfWidth = PitchWidthCm * 0.5f;

    for (const int32 XSide : { -1, 1 })
    {
        for (const int32 YSide : { -1, 1 })
        {
            const FVector2D Corner(
                static_cast<float>(XSide) * (HalfLength + 3600.0f),
                static_cast<float>(YSide) * (HalfWidth + 3200.0f)
            );

            AddBox(
                FName(*FString::Printf(TEXT("FloodlightTower_%d_%d"), XSide, YSide)),
                FVector(Corner.X, Corner.Y, 1550.0f),
                FVector(100.0f, 100.0f, 3100.0f),
                FRotator::ZeroRotator,
                StructureMaterial
            );

            AddBox(
                FName(*FString::Printf(TEXT("FloodlightPanel_%d_%d"), XSide, YSide)),
                FVector(Corner.X, Corner.Y, 3150.0f),
                FVector(350.0f, 700.0f, 90.0f),
                FRotator::ZeroRotator,
                GoalMaterial
            );
        }
    }
}

void AFootballStadiumActor::BuildCircle(
    const FName Prefix,
    const FVector2D& Center,
    const float RadiusCm,
    const int32 Segments
)
{
    if (Segments < 8 || RadiusCm <= 0.0f)
    {
        return;
    }

    const float StepRadians = 2.0f * PI / static_cast<float>(Segments);

    for (int32 Index = 0; Index < Segments; ++Index)
    {
        const float StartAngle = static_cast<float>(Index) * StepRadians;
        const float EndAngle = static_cast<float>(Index + 1) * StepRadians;
        const float MidAngle = (StartAngle + EndAngle) * 0.5f;

        const FVector2D MidPoint(
            Center.X + RadiusCm * FMath::Cos(MidAngle),
            Center.Y + RadiusCm * FMath::Sin(MidAngle)
        );

        const float ChordLength = 2.0f * RadiusCm * FMath::Sin(StepRadians * 0.5f) + 8.0f;
        const float TangentYawDegrees = FMath::RadiansToDegrees(MidAngle) + 90.0f;

        AddBox(
            FName(*FString::Printf(TEXT("%s_%02d"), *Prefix.ToString(), Index)),
            FVector(MidPoint.X, MidPoint.Y, 2.0f),
            FVector(ChordLength, FootballStadium::LineWidthCm, FootballStadium::LineHeightCm),
            FRotator(0.0f, TangentYawDegrees, 0.0f),
            MarkingMaterial
        );
    }
}
