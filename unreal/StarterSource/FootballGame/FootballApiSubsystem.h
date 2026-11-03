#pragma once

#include "CoreMinimal.h"
#include "HttpFwd.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "FootballApiSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFootballApiTextEvent, const FString&, JsonPayload);

/**
 * Minimal HTTP bridge to the local Python catalogue API.
 * This is a runtime C++ subsystem; Unreal Python remains editor-only tooling.
 */
UCLASS()
class FOOTBALLGAME_API UFootballApiSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Set this to the local backend URL without a trailing slash.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Football|API")
    FString ApiBaseUrl = TEXT("http://127.0.0.1:8000");

    UPROPERTY(BlueprintAssignable, Category = "Football|API")
    FFootballApiTextEvent OnClubsReceived;

    UPROPERTY(BlueprintAssignable, Category = "Football|API")
    FFootballApiTextEvent OnRequestFailed;

    UFUNCTION(BlueprintCallable, Category = "Football|API")
    void RequestClubs();

private:
    void HandleClubsResponse(
        FHttpRequestPtr Request,
        FHttpResponsePtr Response,
        bool bWasSuccessful
    );
};
