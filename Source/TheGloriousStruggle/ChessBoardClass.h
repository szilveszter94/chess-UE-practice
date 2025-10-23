#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

    UPROPERTY()
    AChessPieceClass* SelectedPiece;

    UPROPERTY()
    TArray<AChessPieceClass*> Pieces;
};
