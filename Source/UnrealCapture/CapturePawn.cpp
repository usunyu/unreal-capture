// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealCapture.h"
#include "CapturePawn.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "HighResScreenshot.h"

// Sets default values
ACapturePawn::ACapturePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    // Set this pawn to be controlled by the lowest-numbered player
    AutoPossessPlayer = EAutoReceiveInput::Player0;
    // Set capture frame size
    FrameWidth = 1280;
    FrameHeight = 720;
    // Our root component will be a sphere that reacts to physics
    USphereComponent* SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
    RootComponent = SphereComponent;
    SphereComponent->InitSphereRadius(10.0f);
    // Create and position a mesh component so we can see where our sphere is
    UStaticMeshComponent* SphereVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualRepresentation"));
    SphereVisual->SetupAttachment(RootComponent);
    // Create capture component and setup render target
    CaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureComponent"));
    CaptureComponent->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void ACapturePawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACapturePawn::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

}

// Called to bind functionality to input
void ACapturePawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);
    // Bind custom action
    InputComponent->BindAction("Screenshot", IE_Pressed, this, &ACapturePawn::Screenshot);
}

void ACapturePawn::Screenshot() {
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Screenshot"));
    // Get render target
    FRenderTarget *RenderTarget = CaptureComponent->TextureTarget->GameThread_GetRenderTargetResource ();
    // Read pixels
    TArray<FColor> PixelData;
    FReadSurfaceDataFlags ReadSurfaceDataFlags;
    RenderTarget->ReadPixels (PixelData, ReadSurfaceDataFlags, FIntRect (0, 0, FrameWidth, FrameHeight));
    // Save image
    FIntPoint DestSize(FrameWidth, FrameHeight);
    TArray<uint8> CompressedBitmap;
    FImageUtils::CompressImageArray(DestSize.X, DestSize.Y, PixelData, CompressedBitmap);
    FString Filename = "/Users/sun/Desktop/Screenshot.png"; // Your save file path
    FFileHelper::SaveArrayToFile(CompressedBitmap, *Filename);
}

void ACapturePawn::ReadPixels() {
    // Get texture render target pixel data
    FRenderTarget* RenderTarget = CaptureComponent->TextureTarget->GameThread_GetRenderTargetResource();
    FReadSurfaceDataFlags ReadSurfaceDataFlags;
    TArray<FColor> PixelData;
    RenderTarget->ReadPixels (PixelData, ReadSurfaceDataFlags, FIntRect (0, 0, FrameWidth, FrameHeight));
}

void ACapturePawn::ReadPixelsAsync() {
    
}
