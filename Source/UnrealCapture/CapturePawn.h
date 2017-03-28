// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Pawn.h"
#include "CapturePawn.generated.h"

UCLASS()
class UNREALCAPTURE_API ACapturePawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACapturePawn();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

private:
    // Frame width
    int32 FrameWidth;
    // Frame height
    int32 FrameHeight;
	
//    UPROPERTY(EditAnywhere)
    USceneCaptureComponent2D* CaptureComponent;
    
    void Screenshot();
    
    float ReadPixelsTimeWaited;
    TArray<TArray<FColor>> PixelImages;
    TArray<FColor> PixelData;
    void ReadPixels();
    void ReadPixelsAsync();
    
    bool bPixelDataReady;
    bool bWaitingOnPixelData;
};
