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
    
    // Called in several places to guarantee the life of the Actor is coming to an end
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
#if PLATFORM_WINDOWS
    // Import DLL
    static bool ImportDLL(FString folder, FString name);
    
    // Import initialize method
    static bool ImportMethodInitialize();
    
    // Import audioEncoding method
    static bool ImportMethodAudioEncoding();
    
    // Import stopEncoding method
    static bool ImportMethodStopEncoding();
    
    // Import releaseLibResources method
    static bool ImportMethodReleaseLibResources();
    
    // Free DLL
    static void FreeDLL();
#endif
    
private:
    // Frame width
    int32 FrameWidth;
    // Frame height
    int32 FrameHeight;
    
    UPROPERTY(EditAnywhere)
    USceneCaptureComponent2D* CaptureComponent;
    
    FString GetGameDir();
    
    // Screenshot
    void Screenshot();
    
    float ReadPixelsTimeWaited;
    TArray<TArray<FColor>> PixelImages;
    TArray<FColor> PixelData;
    void ReadPixels();
    void ReadPixelsAsync();
    
    bool bPixelDataReady;
    bool bWaitingOnPixelData;
    
#if PLATFORM_WINDOWS
    // Audio capture
    // Currently only work for Windows
    bool bAudioCaptureStart;
    bool bAudioCaptureNeedStop;
    
    void AudioCapture();
#endif
};
