#include "ChessPieceClass.h"
#include "ChessBoardClass.h"
#include "EngineUtils.h"

AChessPieceClass::AChessPieceClass()
{
    PrimaryActorTick.bCanEverTick = false;
}

void AChessPieceClass::BeginPlay()
{
    Super::BeginPlay();
    OnClicked.AddDynamic(this, &AChessPieceClass::HandleClicked);
}

void AChessPieceClass::Tick(float DeltaTime)
{
}

void AChessPieceClass::HandleClicked(AActor* TouchedActor, FKey ButtonPressed)
{
    UE_LOG(LogTemp, Warning, TEXT("Chess piece clicked!"));

    // Find the board
    AChessBoardClass* Board = nullptr;
    for (TActorIterator<AChessBoardClass> It(GetWorld()); It; ++It)
    {
        Board = *It;
        break;
    }
    if (!Board) return;

    if (!Board->SelectedPiece || Board->SelectedPiece->PieceColor == this->PieceColor)
    {
        Board->SelectedPiece = this;
        UE_LOG(LogTemp, Warning, TEXT("Piece selected: %s"), *GetName());
    }
    else
    {
        FVector ClickLocation = GetActorLocation();
        Board->HandleTileClick(ClickLocation, this);
    }
}
