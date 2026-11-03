#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FootballMatchHUDWidget.generated.h"

class UCanvasPanel;
class UTextBlock;
class AFootballMatchDirector;

/** Lightweight native scoreboard. A project may subclass it to replace the visual design. */
UCLASS(Blueprintable)
class FOOTBALLGAME_API UFootballMatchHUDWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
    UPROPERTY(Transient)
    TObjectPtr<UCanvasPanel> RootCanvas = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> ScoreText = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> ClockText = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UTextBlock> PeriodText = nullptr;

    TWeakObjectPtr<AFootballMatchDirector> CachedMatchDirector;

    FString GetPeriodLabel() const;
};
