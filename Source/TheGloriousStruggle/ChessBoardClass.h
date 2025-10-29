#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChessPieceClass.h"
#include "ChessBoardClass.generated.h"

// Forward declare chess piece
class AChessPieceClass;

UCLASS()
class THEGLORIOUSSTRUGGLE_API AChessBoardClass : public AActor
{
    GENERATED_BODY()

public:
    AChessBoardClass();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UFUNCTION()
    void HandleClicked(AActor* TouchedActor, FKey ButtonPressed);
    void HandleTileClick(FVector ClickLocation, AChessPieceClass* TargetPiece);
    void DistributeMouseClick(UStaticMeshComponent* ClickedTile, int32 TileRow, int32 TileCol);

    UPROPERTY()
    AChessPieceClass* SelectedPiece;

    UPROPERTY()
    TArray<AChessPieceClass*> PieceList;

    TArray<TArray<AChessPieceClass*>> PieceMatrix;
    TArray<TArray<UStaticMeshComponent*>> TileMatrix;
    bool IsWhiteTurn;

private:
    void SetClickedTile(
        const FVector& ClickLocation,
        UStaticMeshComponent*& OutClickedTile,
        int32& OutRow,
        int32& OutCol);
    bool MovePieceToPosition(UStaticMeshComponent* ClickedTile, int32 TileRow, int32 TileCol, bool bTake);
    void SetPieceColor(AChessPieceClass* ChessPiece);
    void HandleBishopMove(int32 TileRow, int32 TileCol, UStaticMeshComponent* ClickedTile);
    void HandleKnightMove(int32 TileRow, int32 TileCol, UStaticMeshComponent* ClickedTile);
    void HandleKingMove(int32 TileRow, int32 TileCol, UStaticMeshComponent* ClickedTile);
    void HandleQueenMove(int32 TileRow, int32 TileCol, UStaticMeshComponent* ClickedTile);
    void HandleRookMove(int32 TileRow, int32 TileCol, UStaticMeshComponent* ClickedTile);
    void HandlePawnMove(int32 TileRow, int32 TileCol, UStaticMeshComponent* ClickedTile);
};
