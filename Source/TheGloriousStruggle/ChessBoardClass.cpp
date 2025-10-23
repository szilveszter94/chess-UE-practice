#include "ChessBoardClass.h"
#include "ChessPieceClass.h"
#include "EngineUtils.h"

AChessBoardClass::AChessBoardClass()
{
    PrimaryActorTick.bCanEverTick = false;
    SelectedPiece = nullptr;
}

void AChessBoardClass::BeginPlay()
{
    Super::BeginPlay();
    OnClicked.AddDynamic(this, &AChessBoardClass::HandleClicked);

    for (TActorIterator<AChessPieceClass> It(GetWorld()); It; ++It)
    {
        Pieces.Add(*It);
    }
}

void AChessBoardClass::Tick(float DeltaTime)
{
}

void AChessBoardClass::HandleClicked(AActor* TouchedActor, FKey ButtonPressed)
{
    if (!SelectedPiece) return;

    // Get the mouse click location
    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    FHitResult Hit;
    PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit);

    if (Hit.bBlockingHit)
    {
        FVector ClickLocation = Hit.ImpactPoint;

        // Move the selected piece there
        SelectedPiece->SetActorLocation(ClickLocation);
        UE_LOG(LogTemp, Warning, TEXT("Moved selected piece to: %s"), *ClickLocation.ToString());

        SelectedPiece = nullptr; // unselect
    }
}
