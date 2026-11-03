#include "FootballMatchHUDWidget.h"

#include "FootballMatchDirector.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/TextBlock.h"
#include "EngineUtils.h"

void UFootballMatchHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (!WidgetTree)
    {
        return;
    }

    RootCanvas = WidgetTree->ConstructWidget<UCanvasPanel>(
        UCanvasPanel::StaticClass(), TEXT("FootballMatchHUDRoot")
    );
    WidgetTree->RootWidget = RootCanvas;

    auto AddHUDText = [this](const FName Name, const FVector2D Position,
        const FVector2D Size, const int32 FontSize) -> UTextBlock*
    {
        UTextBlock* Text = WidgetTree->ConstructWidget<UTextBlock>(UTextBlock::StaticClass(), Name);
        Text->SetJustification(ETextJustify::Center);
        Text->SetColorAndOpacity(FSlateColor(FLinearColor::White));
        Text->SetShadowColorAndOpacity(FLinearColor(0.0f, 0.0f, 0.0f, 0.85f));
        Text->SetShadowOffset(FVector2D(1.5f, 1.5f));
        FSlateFontInfo Font = Text->GetFont();
        Font.Size = FontSize;
        Text->SetFont(Font);

        UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(RootCanvas->AddChild(Text));
        if (Slot)
        {
            Slot->SetAnchors(FAnchors(0.5f, 0.0f, 0.5f, 0.0f));
            Slot->SetAlignment(FVector2D(0.5f, 0.0f));
            Slot->SetPosition(Position);
            Slot->SetSize(Size);
        }
        return Text;
    };

    ScoreText = AddHUDText(TEXT("ScoreText"), FVector2D(0.0f, 18.0f), FVector2D(460.0f, 42.0f), 26);
    ClockText = AddHUDText(TEXT("ClockText"), FVector2D(0.0f, 58.0f), FVector2D(260.0f, 30.0f), 19);
    PeriodText = AddHUDText(TEXT("PeriodText"), FVector2D(0.0f, 88.0f), FVector2D(260.0f, 24.0f), 12);

    if (ScoreText)
    {
        ScoreText->SetText(FText::FromString(TEXT("TEAM 0   0 - 0   TEAM 1")));
    }
    if (ClockText)
    {
        ClockText->SetText(FText::FromString(TEXT("00:00")));
    }
}

void UFootballMatchHUDWidget::NativeTick(
    const FGeometry& MyGeometry,
    const float InDeltaTime
)
{
    Super::NativeTick(MyGeometry, InDeltaTime);

    if (!CachedMatchDirector.IsValid() && GetWorld())
    {
        for (TActorIterator<AFootballMatchDirector> It(GetWorld()); It; ++It)
        {
            CachedMatchDirector = *It;
            break;
        }
    }

    AFootballMatchDirector* Director = CachedMatchDirector.Get();
    if (!IsValid(Director))
    {
        return;
    }

    if (ScoreText)
    {
        ScoreText->SetText(FText::FromString(FString::Printf(
            TEXT("TEAM 0   %d - %d   TEAM 1"),
            Director->TeamZeroScore,
            Director->TeamOneScore
        )));
    }
    if (ClockText)
    {
        ClockText->SetText(FText::FromString(Director->GetMatchClockText()));
    }
    if (PeriodText)
    {
        PeriodText->SetText(FText::FromString(GetPeriodLabel()));
    }
}

FString UFootballMatchHUDWidget::GetPeriodLabel() const
{
    const AFootballMatchDirector* Director = CachedMatchDirector.Get();
    if (!IsValid(Director))
    {
        return TEXT("MATCH");
    }

    switch (Director->CurrentPeriod)
    {
    case EFootballMatchPeriod::PreMatch: return TEXT("PRE-MATCH");
    case EFootballMatchPeriod::FirstHalf: return TEXT("FIRST HALF");
    case EFootballMatchPeriod::HalfTime: return TEXT("HALF-TIME");
    case EFootballMatchPeriod::SecondHalf: return TEXT("SECOND HALF");
    case EFootballMatchPeriod::FullTime: return TEXT("FULL-TIME");
    default: return TEXT("MATCH");
    }
}
