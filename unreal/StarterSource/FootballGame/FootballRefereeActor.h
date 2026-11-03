#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FootballRefereeActor.generated.h"

class AFootballBallActor;
class UStaticMeshComponent;
class USceneComponent;

/** Visual placeholder referee that keeps a readable running position near play. */
UCLASS(Blueprintable)
class FOOTBALLGAME_API AFootballRefereeActor : public AActor
{
    GENERATED_BODY()

public:
    AFootballRefereeActor();
    virtual void Tick(float DeltaSeconds) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Referee")
    TObjectPtr<AFootballBallActor> TrackedBall = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Referee")
    FVector FollowOffsetCm = FVector(-520.0f, -820.0f, 0.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Referee", meta = (ClampMin = "0.1"))
    float FollowInterpSpeed = 2.6f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Referee", meta = (ClampMin = "0.1"))
    float FacingInterpSpeed = 5.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|Referee")
    bool bAutoTrackBall = true;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere, Category = "Football|Referee")
    TObjectPtr<USceneComponent> SceneRoot = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Football|Referee")
    TObjectPtr<UStaticMeshComponent> BodyProxy = nullptr;

    UPROPERTY(VisibleAnywhere, Category = "Football|Referee")
    TObjectPtr<UStaticMeshComponent> HeadProxy = nullptr;

    float BallSearchAccumulator = 0.0f;
    void FindTrackedBall();
};
