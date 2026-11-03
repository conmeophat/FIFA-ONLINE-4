#include "FootballApiSubsystem.h"

#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"

void UFootballApiSubsystem::RequestClubs()
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request =
        FHttpModule::Get().CreateRequest();

    Request->SetURL(ApiBaseUrl + TEXT("/api/v1/clubs"));
    Request->SetVerb(TEXT("GET"));
    Request->SetHeader(TEXT("Accept"), TEXT("application/json"));
    Request->SetTimeout(10.0f);

    Request->OnProcessRequestComplete().BindUObject(
        this,
        &UFootballApiSubsystem::HandleClubsResponse
    );

    if (!Request->ProcessRequest())
    {
        OnRequestFailed.Broadcast(TEXT("Could not start HTTP request."));
    }
}

void UFootballApiSubsystem::HandleClubsResponse(
    FHttpRequestPtr Request,
    FHttpResponsePtr Response,
    bool bWasSuccessful
)
{
    (void)Request;

    if (!bWasSuccessful || !Response.IsValid())
    {
        OnRequestFailed.Broadcast(TEXT("Backend request failed or timed out."));
        return;
    }

    const int32 StatusCode = Response->GetResponseCode();
    if (StatusCode < 200 || StatusCode >= 300)
    {
        OnRequestFailed.Broadcast(
            FString::Printf(TEXT("Backend returned HTTP %d."), StatusCode)
        );
        return;
    }

    OnClubsReceived.Broadcast(Response->GetContentAsString());
}
