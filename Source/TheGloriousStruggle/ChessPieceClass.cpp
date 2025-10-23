#include "ChessPieceClass.h"
#include "ChessBoardClass.h" // full class needed for TActorIterator
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

    // Notify the board
    for (TActorIterator<AChessBoardClass> It(GetWorld()); It; ++It)
    {
        AChessBoardClass* Board = *It;
        if (Board)
        {
            Board->SelectedPiece = this;
            break;
        }
    }
}
