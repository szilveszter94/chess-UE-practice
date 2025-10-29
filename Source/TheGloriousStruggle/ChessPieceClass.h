#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChessPieceClass.generated.h"

// Forward declare board
class AChessBoardClass;

UENUM(BlueprintType)
enum class EPieceColor : uint8
{
    None,
    White,
    Black
};

UENUM(BlueprintType)
enum class EPieceType : uint8
{
    King,
    Queen,
    Rook,
    Bishop,
    Knight,
    Pawn
};

UCLASS()
class THEGLORIOUSSTRUGGLE_API AChessPieceClass : public AActor
{
    GENERATED_BODY()

public:
    AChessPieceClass();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    UFUNCTION()
    void HandleClicked(AActor* TouchedActor, FKey ButtonPressed);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Chess")
    EPieceType PieceType;

    int32 Row;
    int32 Col;
    EPieceColor PieceColor;
};
