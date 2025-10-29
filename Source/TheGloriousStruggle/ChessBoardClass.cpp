#include "ChessBoardClass.h"
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
    IsWhiteTurn = true;

    for (TActorIterator<AChessPieceClass> It(GetWorld()); It; ++It)
    {
        PieceList.Add(*It);
    }

    TArray<UStaticMeshComponent*> Components;
    GetComponents<UStaticMeshComponent>(Components);

    TArray<UStaticMeshComponent*> SortedTiles = Components;

    SortedTiles.Sort([](const UStaticMeshComponent& A, const UStaticMeshComponent& B) {
        FVector LocA = A.GetComponentLocation();
        FVector LocB = B.GetComponentLocation();

        if (!FMath::IsNearlyEqual(LocA.Y, LocB.Y))
            return LocA.Y > LocB.Y;

        return LocA.X < LocB.X;
        });

    int32 Index = 0;
    for (int32 Row = 0; Row < 8; ++Row)
    {
        TArray<UStaticMeshComponent*> RowTiles;
        TArray<AChessPieceClass*> RowPieces;
        for (int32 Col = 0; Col < 8; ++Col)
        {
            if (Index < SortedTiles.Num())
            {
                UStaticMeshComponent* Tile = SortedTiles[Index];
                RowTiles.Add(Tile);

                AChessPieceClass* FoundPiece = nullptr;
                for (AChessPieceClass* Piece : PieceList)
                {
                    if (Piece)
                    {
                        FVector PieceLoc = Piece->GetActorLocation();
                        FVector TileLoc = Tile->GetComponentLocation();

                        float Dist = FVector::Dist2D(PieceLoc, TileLoc);
                        if (Dist < 50.f) // small threshold — tweak if needed
                        {
                            FoundPiece = Piece;
                            break;
                        }
                    }
                }

                RowPieces.Add(FoundPiece);
                if (FoundPiece) {
                    SetPieceColor(FoundPiece);
                    FoundPiece->Row = Row;
                    FoundPiece->Col = Col;
                }
                Index++;
            }
        }
        TileMatrix.Add(RowTiles);
        PieceMatrix.Add(RowPieces);
    }
}

void AChessBoardClass::Tick(float DeltaTime)
{
}

void AChessBoardClass::HandleClicked(AActor* TouchedActor, FKey ButtonPressed)
{
    if (!SelectedPiece) return;

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) return;

    FHitResult Hit;
    PC->GetHitResultUnderCursor(ECC_Visibility, false, Hit);

    if (!Hit.bBlockingHit) return;

    FVector ClickLocation = Hit.ImpactPoint;
    UStaticMeshComponent* ClickedTile = nullptr;
    int32 TileRow = -1;
    int32 TileCol = -1;

    SetClickedTile(ClickLocation, ClickedTile, TileRow, TileCol);
    DistributeMouseClick(ClickedTile, TileRow, TileCol);
}

void AChessBoardClass::HandleTileClick(FVector ClickLocation, AChessPieceClass* TargetPiece = nullptr)
{
    int32 TileRow = -1;
    int32 TileCol = -1;
    UStaticMeshComponent* ClickedTile = nullptr;

    SetClickedTile(ClickLocation, ClickedTile, TileRow, TileCol);
    if (!SelectedPiece) return;

    DistributeMouseClick(ClickedTile, TileRow, TileCol);
}

void AChessBoardClass::DistributeMouseClick(UStaticMeshComponent* ClickedTile, int32 TileRow, int32 TileCol) {
    if (ClickedTile && TileRow != -1 && TileCol != -1)
    {
        EPieceType PieceType = SelectedPiece->PieceType;

        switch (PieceType)
        {
        case EPieceType::Bishop:
            HandleBishopMove(TileRow, TileCol, ClickedTile);
            break;
        case EPieceType::Rook:
            HandleRookMove(TileRow, TileCol, ClickedTile);
            break;
        case EPieceType::King:
            HandleKingMove(TileRow, TileCol, ClickedTile);
            break;
        case EPieceType::Queen:
            HandleQueenMove(TileRow, TileCol, ClickedTile);
            break;
        case EPieceType::Pawn:
            HandlePawnMove(TileRow, TileCol, ClickedTile);
            break;
        case EPieceType::Knight:
            HandleKnightMove(TileRow, TileCol, ClickedTile);
            break;

        default:
            break;
        }

        SelectedPiece = nullptr; // unselect
    }
}

void AChessBoardClass::SetPieceColor(AChessPieceClass* ChessPiece) {
    UStaticMeshComponent* MeshComp = ChessPiece->FindComponentByClass<UStaticMeshComponent>();
    if (MeshComp)
    {
        UMaterialInterface* Material = MeshComp->GetMaterial(0);
        if (Material)
        {
            FString MaterialName = Material->GetName();

            if (MaterialName.Contains("White", ESearchCase::IgnoreCase))
            {
                ChessPiece->PieceColor = EPieceColor::White;
                return;
            }
            else if (MaterialName.Contains("Black", ESearchCase::IgnoreCase))
            {
                ChessPiece->PieceColor = EPieceColor::Black;
                return;
            }
        }
    }

    ChessPiece->PieceColor = EPieceColor::None;
}

void AChessBoardClass::SetClickedTile(
    const FVector& ClickLocation,
    UStaticMeshComponent*& OutClickedTile,
    int32& OutRow,
    int32& OutCol)
{
    OutClickedTile = nullptr;
    OutRow = -1;
    OutCol = -1;

    for (int32 Row = 0; Row < TileMatrix.Num(); ++Row)
    {
        for (int32 Col = 0; Col < TileMatrix[Row].Num(); ++Col)
        {
            UStaticMeshComponent* Tile = TileMatrix[Row][Col];
            if (!Tile) continue;

            // Simple bounds check
            FVector TileLocation = Tile->GetComponentLocation();
            FVector TileExtent = Tile->Bounds.BoxExtent; // half size of tile

            if (FMath::Abs(ClickLocation.X - TileLocation.X) <= TileExtent.X &&
                FMath::Abs(ClickLocation.Y - TileLocation.Y) <= TileExtent.Y)
            {
                OutClickedTile = Tile;
                OutRow = Row;
                OutCol = Col;
                return;
            }
        }
    }
}

bool AChessBoardClass::MovePieceToPosition(UStaticMeshComponent* ClickedTile, int32 TileRow, int32 TileCol, bool bTake = false)
{
    if ((SelectedPiece->PieceColor == EPieceColor::White && !IsWhiteTurn) || (SelectedPiece->PieceColor == EPieceColor::Black && IsWhiteTurn)) {
        UE_LOG(LogTemp, Warning, TEXT("Invalid move: Opposite color's turn"));
        return false;
    }
    if (!SelectedPiece) return false;

    if (bTake && PieceMatrix[TileRow][TileCol])
    {
        AChessPieceClass* TargetPiece = PieceMatrix[TileRow][TileCol];
        if (TargetPiece->PieceType == EPieceType::King) {
            UE_LOG(LogTemp, Warning, TEXT("Invalid move: King cannot be taken"));
            return false;
        }

        if (TargetPiece->IsValidLowLevel())
        {
            TargetPiece->Destroy();
        }

        PieceMatrix[TileRow][TileCol] = nullptr;
    }

    int32 PrevRow = SelectedPiece->Row;
    int32 PrevCol = SelectedPiece->Col;
    PieceMatrix[PrevRow][PrevCol] = nullptr;

    FVector TileCenter = ClickedTile->GetComponentLocation();
    SelectedPiece->SetActorLocation(TileCenter);

    SelectedPiece->Row = TileRow;
    SelectedPiece->Col = TileCol;

    PieceMatrix[TileRow][TileCol] = SelectedPiece;
    IsWhiteTurn = !IsWhiteTurn;
    return true;
}

void AChessBoardClass::HandleBishopMove(int32 TileRow, int32 TileCol, UStaticMeshComponent* ClickedTile) {
    int32 PieceRow = SelectedPiece->Row;
    int32 PieceCol = SelectedPiece->Col;

    UE_LOG(LogTemp, Warning, TEXT("Move from [%d][%d] to [%d][%d]"),
        PieceRow, PieceCol, TileRow, TileCol);

    // 1️ Check if move is diagonal
    int32 DeltaRow = TileRow - PieceRow;
    int32 DeltaCol = TileCol - PieceCol;

    if (FMath::Abs(DeltaRow) != FMath::Abs(DeltaCol))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid move: not diagonal"));
        return;
    }

    // 2️ Determine step direction
    int32 StepRow = (DeltaRow > 0) ? 1 : -1;
    int32 StepCol = (DeltaCol > 0) ? 1 : -1;

    int32 CurrentRow = PieceRow + StepRow;
    int32 CurrentCol = PieceCol + StepCol;

    // 3️ Check path (all tiles along diagonal must be empty)
    while (CurrentRow != TileRow && CurrentCol != TileCol)
    {
        AChessPieceClass* PieceOnTile = PieceMatrix[CurrentRow][CurrentCol];
        if (PieceOnTile)
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid move: path blocked at [%d][%d]"),
                CurrentRow, CurrentCol);
            return;
        }

        CurrentRow += StepRow;
        CurrentCol += StepCol;
    }

    // 4️ Finally, check the target tile is empty
    AChessPieceClass* TargetPiece = PieceMatrix[TileRow][TileCol];
    if (TargetPiece)
    {
        if (TargetPiece->PieceColor == SelectedPiece->PieceColor)
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid move: target tile is an ally piece"));
            return;
        }
        else
        {
            bool isValid = MovePieceToPosition(ClickedTile, TileRow, TileCol, true);
            if (isValid) UE_LOG(LogTemp, Warning, TEXT("Knight takes piece at [%d][%d]!"), TileRow, TileCol);
            return;
        }
    }

    bool isValid = MovePieceToPosition(ClickedTile, TileRow, TileCol);
    if (isValid) UE_LOG(LogTemp, Warning, TEXT("Valid bishop move!"));
}

void AChessBoardClass::HandleKnightMove(int32 TileRow, int32 TileCol, UStaticMeshComponent* ClickedTile)
{
    int32 PieceRow = SelectedPiece->Row;
    int32 PieceCol = SelectedPiece->Col;

    UE_LOG(LogTemp, Warning, TEXT("Knight move from [%d][%d] to [%d][%d]"),
        PieceRow, PieceCol, TileRow, TileCol);

    // Compute row and col difference
    int32 DeltaRow = FMath::Abs(TileRow - PieceRow);
    int32 DeltaCol = FMath::Abs(TileCol - PieceCol);

    // Check for valid L-shape move
    if (!((DeltaRow == 2 && DeltaCol == 1) || (DeltaRow == 1 && DeltaCol == 2)))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid move: not L-shape for Knight"));
        return;
    }

    // Check target tile
    AChessPieceClass* TargetPiece = PieceMatrix[TileRow][TileCol];
    if (TargetPiece)
    {
        if (TargetPiece->PieceColor == SelectedPiece->PieceColor)
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid move: target tile is an ally piece"));
            return;
        }
        else
        {
            // Take the piece
            bool isValid = MovePieceToPosition(ClickedTile, TileRow, TileCol, true);
            if (isValid) UE_LOG(LogTemp, Warning, TEXT("Knight takes piece at [%d][%d]!"), TileRow, TileCol);
            return;
        }
    }

    // Move to empty tile
    bool isValid = MovePieceToPosition(ClickedTile, TileRow, TileCol);
    UE_LOG(LogTemp, Warning, TEXT("Valid Knight move to [%d][%d]!"), TileRow, TileCol);
}

void AChessBoardClass::HandleKingMove(int32 TileRow, int32 TileCol, UStaticMeshComponent* ClickedTile)
{
    int32 PieceRow = SelectedPiece->Row;
    int32 PieceCol = SelectedPiece->Col;

    UE_LOG(LogTemp, Warning, TEXT("King move from [%d][%d] to [%d][%d]"),
        PieceRow, PieceCol, TileRow, TileCol);

    // Check if move is at most 1 tile in any direction
    int32 DeltaRow = FMath::Abs(TileRow - PieceRow);
    int32 DeltaCol = FMath::Abs(TileCol - PieceCol);

    if (DeltaRow > 1 || DeltaCol > 1 || (DeltaRow == 0 && DeltaCol == 0))
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid move: King can only move 1 tile"));
        return;
    }

    // Check target tile
    AChessPieceClass* TargetPiece = PieceMatrix[TileRow][TileCol];
    if (TargetPiece)
    {
        if (TargetPiece->PieceColor == SelectedPiece->PieceColor)
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid move: target tile is an ally piece"));
            return;
        }
        else
        {
            // Take the piece
            bool isValid = MovePieceToPosition(ClickedTile, TileRow, TileCol, true);
            if (isValid) UE_LOG(LogTemp, Warning, TEXT("King takes piece at [%d][%d]!"), TileRow, TileCol);
            return;
        }
    }

    // Move to empty tile
    bool isValid = MovePieceToPosition(ClickedTile, TileRow, TileCol);
    if (isValid) UE_LOG(LogTemp, Warning, TEXT("Valid King move to [%d][%d]!"), TileRow, TileCol);
}

void AChessBoardClass::HandleQueenMove(int32 TileRow, int32 TileCol, UStaticMeshComponent* ClickedTile)
{
    int32 PieceRow = SelectedPiece->Row;
    int32 PieceCol = SelectedPiece->Col;

    UE_LOG(LogTemp, Warning, TEXT("Queen move from [%d][%d] to [%d][%d]"),
        PieceRow, PieceCol, TileRow, TileCol);

    int32 DeltaRow = TileRow - PieceRow;
    int32 DeltaCol = TileCol - PieceCol;

    bool bIsDiagonal = FMath::Abs(DeltaRow) == FMath::Abs(DeltaCol);
    bool bIsStraight = (DeltaRow == 0 && DeltaCol != 0) || (DeltaCol == 0 && DeltaRow != 0);

    if (!bIsDiagonal && !bIsStraight)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid move: Queen must move straight or diagonal"));
        return;
    }

    // Determine step
    int32 StepRow = (DeltaRow == 0) ? 0 : (DeltaRow > 0 ? 1 : -1);
    int32 StepCol = (DeltaCol == 0) ? 0 : (DeltaCol > 0 ? 1 : -1);

    int32 CurrentRow = PieceRow + StepRow;
    int32 CurrentCol = PieceCol + StepCol;

    // Check path (exclude destination)
    while (CurrentRow != TileRow || CurrentCol != TileCol)
    {
        if (PieceMatrix[CurrentRow][CurrentCol])
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid move: path blocked at [%d][%d]"), CurrentRow, CurrentCol);
            return;
        }

        CurrentRow += StepRow;
        CurrentCol += StepCol;
    }

    // Check destination tile
    AChessPieceClass* TargetPiece = PieceMatrix[TileRow][TileCol];
    if (TargetPiece)
    {
        if (TargetPiece->PieceColor == SelectedPiece->PieceColor)
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid move: target tile is an ally piece"));
            return;
        }
        else
        {
            bool isValid = MovePieceToPosition(ClickedTile, TileRow, TileCol, true);
            if (isValid) UE_LOG(LogTemp, Warning, TEXT("Queen takes piece at [%d][%d]!"), TileRow, TileCol);
            return;
        }
    }

    bool isValid = MovePieceToPosition(ClickedTile, TileRow, TileCol);
    if (isValid) UE_LOG(LogTemp, Warning, TEXT("Valid Queen move to [%d][%d]!"), TileRow, TileCol);
}

void AChessBoardClass::HandleRookMove(int32 TileRow, int32 TileCol, UStaticMeshComponent* ClickedTile)
{
    int32 PieceRow = SelectedPiece->Row;
    int32 PieceCol = SelectedPiece->Col;

    UE_LOG(LogTemp, Warning, TEXT("Rook move from [%d][%d] to [%d][%d]"),
        PieceRow, PieceCol, TileRow, TileCol);

    int32 DeltaRow = TileRow - PieceRow;
    int32 DeltaCol = TileCol - PieceCol;

    // Must move strictly horizontal or vertical
    if (DeltaRow != 0 && DeltaCol != 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("Invalid move: Rook must move straight"));
        return;
    }

    int32 StepRow = (DeltaRow == 0) ? 0 : (DeltaRow > 0 ? 1 : -1);
    int32 StepCol = (DeltaCol == 0) ? 0 : (DeltaCol > 0 ? 1 : -1);

    int32 CurrentRow = PieceRow + StepRow;
    int32 CurrentCol = PieceCol + StepCol;

    // Check path (exclude destination)
    while (CurrentRow != TileRow || CurrentCol != TileCol)
    {
        if (PieceMatrix[CurrentRow][CurrentCol])
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid move: path blocked at [%d][%d]"),
                CurrentRow, CurrentCol);
            return;
        }

        CurrentRow += StepRow;
        CurrentCol += StepCol;
    }

    // Check destination
    AChessPieceClass* TargetPiece = PieceMatrix[TileRow][TileCol];
    if (TargetPiece)
    {
        if (TargetPiece->PieceColor == SelectedPiece->PieceColor)
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid move: target tile is an ally piece"));
            return;
        }
        else
        {
            bool isValid = MovePieceToPosition(ClickedTile, TileRow, TileCol, true);
            if (isValid) UE_LOG(LogTemp, Warning, TEXT("Rook takes piece at [%d][%d]!"), TileRow, TileCol);
            return;
        }
    }

    bool isValid = MovePieceToPosition(ClickedTile, TileRow, TileCol);
    UE_LOG(LogTemp, Warning, TEXT("Valid Rook move to [%d][%d]!"), TileRow, TileCol);
}

void AChessBoardClass::HandlePawnMove(int32 TileRow, int32 TileCol, UStaticMeshComponent* ClickedTile) {
    int32 PieceRow = SelectedPiece->Row;
    int32 PieceCol = SelectedPiece->Col;
    EPieceColor Color = SelectedPiece->PieceColor;

    UE_LOG(LogTemp, Warning, TEXT("Pawn move from [%d][%d] to [%d][%d]"), PieceRow, PieceCol, TileRow, TileCol);

    int32 Direction = (Color == EPieceColor::White) ? 1 : -1; // White moves "up" (increasing row), Black "down" (decreasing row)
    int32 StartRow = (Color == EPieceColor::White) ? 1 : 6;    // starting row for double-step

    int32 DeltaRow = TileRow - PieceRow;
    int32 DeltaCol = TileCol - PieceCol;
    bool bMoved = false;

    // 1️ Normal one-step forward
    if (DeltaCol == 0 && DeltaRow == Direction)
    {
        if (PieceMatrix[TileRow][TileCol] == nullptr)
        {
            bMoved = true;
            bool isValid = MovePieceToPosition(ClickedTile, TileRow, TileCol);
            if (isValid) UE_LOG(LogTemp, Warning, TEXT("Pawn moved one step forward."));
            return;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Pawn blocked forward."));
            return;
        }
    }

    // 2️ Double-step from starting position
    if (DeltaCol == 0 && DeltaRow == 2 * Direction && PieceRow == StartRow)
    {
        if (PieceMatrix[PieceRow + Direction][PieceCol] == nullptr &&
            PieceMatrix[TileRow][TileCol] == nullptr)
        {
            bMoved = true;
            bool isValid = MovePieceToPosition(ClickedTile, TileRow, TileCol);
            if (isValid) UE_LOG(LogTemp, Warning, TEXT("Pawn double-step from start."));
            return;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Pawn blocked for double-step."));
            return;
        }
    }

    // 3️ Diagonal capture
    if (FMath::Abs(DeltaCol) == 1 && DeltaRow == Direction)
    {
        AChessPieceClass* TargetPiece = PieceMatrix[TileRow][TileCol];
        if (TargetPiece && TargetPiece->PieceColor != Color)
        {
            bMoved = true;
            bool isValid = MovePieceToPosition(ClickedTile, TileRow, TileCol, true);
            if (isValid) UE_LOG(LogTemp, Warning, TEXT("Pawn captured piece at [%d][%d]!"), TileRow, TileCol);
            return;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Invalid diagonal move: no enemy piece to capture."));
            return;
        }
    }

    if (!bMoved) {
        UE_LOG(LogTemp, Warning, TEXT("Invalid pawn move."));
        return;
    }

    // Handle pawn promotion
    if ((Color == EPieceColor::White && TileRow == 7) ||
        (Color == EPieceColor::Black && TileRow == 0)) {
        // TODO -- Pawn promotion
    }
}