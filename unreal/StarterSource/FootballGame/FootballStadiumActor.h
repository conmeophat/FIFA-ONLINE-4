#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FootballStadiumActor.generated.h"

class UMaterialInterface;
class USceneComponent;
class UStaticMesh;
class UStaticMeshComponent;

/**
 * Procedural, scale-aware football stadium blockout.
 *
 * Dimensions are in centimetres. The pitch is centred on the actor origin:
 * X = pitch length, Y = pitch width, Z = up.
 * Assign project materials in the Details panel for the intended grass/line/stand look.
 */
UCLASS()
class FOOTBALLGAME_API AFootballStadiumActor : public AActor
{
    GENERATED_BODY()

public:
    AFootballStadiumActor();

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Pitch", meta = (ClampMin = "5000.0"))
    float PitchLengthCm = 10500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Pitch", meta = (ClampMin = "3000.0"))
    float PitchWidthCm = 6800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Pitch", meta = (ClampMin = "500.0"))
    float PenaltyAreaDepthCm = 1650.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Pitch", meta = (ClampMin = "1000.0"))
    float PenaltyAreaWidthCm = 4032.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Pitch")
    bool bBuildStands = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Pitch")
    bool bBuildFloodlightTowers = true;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Materials")
    TObjectPtr<UMaterialInterface> PitchMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Materials")
    TObjectPtr<UMaterialInterface> MarkingMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Materials")
    TObjectPtr<UMaterialInterface> GoalMaterial = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Football|Materials")
    TObjectPtr<UMaterialInterface> StructureMaterial = nullptr;

protected:
    virtual void OnConstruction(const FTransform& Transform) override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Football")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Football")
    TObjectPtr<UStaticMeshComponent> PitchSurface;

    UPROPERTY(Transient)
    TObjectPtr<UStaticMesh> UnitCubeMesh;

    UPROPERTY(Transient)
    TArray<TObjectPtr<UStaticMeshComponent>> GeneratedComponents;

    UStaticMeshComponent* AddBox(
        const FName ComponentName,
        const FVector& CenterCm,
        const FVector& SizeCm,
        const FRotator& Rotation,
        UMaterialInterface* Material,
        bool bEnableCollision = false
    );

    void ClearGeneratedComponents();
    void BuildPitchMarkings();
    void BuildPenaltyAreas();
    void BuildGoals();
    void BuildCornerFlags();
    void BuildStands();
    void BuildFloodlightTowers();
    void BuildCircle(const FName Prefix, const FVector2D& Center, float RadiusCm, int32 Segments);
};
