#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FootballBroadcastCameraActor.generated.h"

class UCameraComponent;
class USceneComponent;

/**
 * High sideline broadcast camera for the football pitch.
 * Tracks a ball/target actor and smoothly pans along the pitch length.
 */
UCLASS()
class FOOTBALLGAME_API AFootballBroadcastCameraActor : public AActor
{
    GENERATED_BODY()

public:
    AFootballBroadcastCameraActor();

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Football|Broadcast")
    TObjectPtr<AActor> TrackedActor = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Broadcast", meta = (ClampMin = "1000.0"))
    float PitchLengthCm = 10500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Broadcast", meta = (ClampMin = "3000.0"))
    float PitchWidthCm = 6800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Broadcast", meta = (ClampMin = "1000.0"))
    float DistanceFromSidelineCm = 6500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Broadcast", meta = (ClampMin = "1000.0"))
    float CameraHeightCm = 5200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Broadcast")
    float CameraLeadCm = 900.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Broadcast")
    float FocusHeightCm = 110.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Broadcast", meta = (ClampMin = "0.1"))
    float PositionInterpSpeed = 2.8f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Broadcast", meta = (ClampMin = "0.1"))
    float RotationInterpSpeed = 4.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Broadcast", meta = (ClampMin = "20.0", ClampMax = "100.0"))
    float FieldOfView = 42.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Broadcast")
    bool bActivateAsPlayerViewOnBeginPlay = true;

    UFUNCTION(BlueprintCallable, Category = "Football|Broadcast")
    void SetTrackedActor(AActor* NewTrackedActor);

    UFUNCTION(BlueprintCallable, Category = "Football|Broadcast")
    void SetManualFocusLocation(const FVector& NewFocusLocation);

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaSeconds) override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Football|Broadcast")
    TObjectPtr<USceneComponent> SceneRoot;

    UPROPERTY(VisibleAnywhere, Category = "Football|Broadcast")
    TObjectPtr<UCameraComponent> BroadcastCamera;

    UPROPERTY(EditAnywhere, Category = "Football|Broadcast")
    FVector ManualFocusLocation = FVector::ZeroVector;

    bool bHasActivatedPlayerView = false;

    FVector GetFocusLocation() const;
    FVector GetDesiredCameraLocation(const FVector& FocusLocation) const;
};
