// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealCapture.h"
#include "CapturePawn.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "HighResScreenshot.h"

// Sets default values
ACapturePawn::ACapturePawn()
{
 	// Set this pawn to call Tick() every frame.
    // You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    // Set this pawn to be controlled by the lowest-numbered player
    AutoPossessPlayer = EAutoReceiveInput::Player0;
    // Set capture frame size
    FrameWidth = 1280;
    FrameHeight = 720;
    // Pixel data check
    bPixelDataReady = false;
    bWaitingOnPixelData = false;
    // Wait seconds then start read pixels
    ReadPixelsTimeWaited = 0;
    // Our root component will be a sphere that reacts to physics
    USphereComponent* SphereComponent =
        CreateDefaultSubobject<USphereComponent>(TEXT("RootComponent"));
    RootComponent = SphereComponent;
    SphereComponent->InitSphereRadius(10.0f);
    // Create and position a mesh component so we can see where our sphere is
    UStaticMeshComponent* SphereVisual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualRepresentation"));
    SphereVisual->SetupAttachment(RootComponent);
    // Create capture component
    CaptureComponent =
        CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("CaptureComponent"));
    CaptureComponent->SetupAttachment(RootComponent);
    CaptureComponent->UpdateBounds();
    CaptureComponent->CaptureSource = SCS_FinalColorLDR;
    // Setup render texture target
    CaptureComponent->TextureTarget = NewObject<UTextureRenderTarget2D>();
    CaptureComponent->TextureTarget->ClearColor = FLinearColor::Black;
    CaptureComponent->TextureTarget->InitCustomFormat(FrameWidth, FrameHeight, PF_B8G8R8A8, true);
    CaptureComponent->TextureTarget->bHDR = 0;
    CaptureComponent->TextureTarget->TargetGamma = 0.9;
    CaptureComponent->TextureTarget->CompressionNoAlpha = true;
    CaptureComponent->TextureTarget->CompressionSettings = TC_VectorDisplacementmap;
    CaptureComponent->TextureTarget->MipGenSettings = TMGS_NoMipmaps;
    
//    TArray<FColor> BlankImage;
//	BlankImage.Empty(FrameWidth * FrameHeight);
//	// Allocate the frames once, allocation can be very slow
//	for (int32 i = 0; PixelImages() < FramesMax && i < FramesMax; i++) {
//		PixelImages.Add(BlankImage);
//	}
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

    if (ReadPixelsTimeWaited < 1.0f) {
		ReadPixelsTimeWaited += DeltaTime;
		return;
	}

    if (bWaitingOnPixelData) {
		if (bPixelDataReady) {
            if (PixelImages.Num() < 1) {
                PixelImages.Add(PixelData);
            } else {
                PixelImages[0] = PixelData;
            }

			bWaitingOnPixelData = false;
			bPixelDataReady = false;
		}
	}
    
    if (!bWaitingOnPixelData) {
        // No ReadPixels stats:
        // FPS:     ~70fps
        // Frame:   ~13ms
    
        // ReadPixelsAsync stats:
        // FPS:     ~30fps
        // Frame:   ~30ms
        ReadPixelsAsync();

        // ReadPixels stats:
        // FPS:     ~24fps
        // Frame:   ~40ms
//        ReadPixels();
    }
}

// Called to bind functionality to input
void ACapturePawn::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);
    // Bind custom action
    InputComponent->BindAction(
        "Screenshot",
        IE_Pressed, this,
        &ACapturePawn::Screenshot);
}

void ACapturePawn::Screenshot() {
    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Screenshot"));
    // Get render target
    FRenderTarget* RenderTarget =
        CaptureComponent->TextureTarget->GameThread_GetRenderTargetResource ();
    // Read pixels
    FReadSurfaceDataFlags ReadSurfaceDataFlags;
    ReadSurfaceDataFlags.SetLinearToGamma(false);
    RenderTarget->ReadPixels(
        PixelData,
        ReadSurfaceDataFlags,
        FIntRect (0, 0, FrameWidth, FrameHeight));
    // Save image
    FIntPoint DestSize(FrameWidth, FrameHeight);
    TArray<uint8> CompressedBitmap;
    FImageUtils::CompressImageArray(
        DestSize.X, DestSize.Y,
        PixelData, CompressedBitmap);
    FString Filename = "/Users/sun/Desktop/Screenshot.png"; // Your save file path
    FFileHelper::SaveArrayToFile(CompressedBitmap, *Filename);
}

void ACapturePawn::ReadPixels() {
    // Get texture render target pixel data
    FRenderTarget* RenderTarget =
        CaptureComponent->TextureTarget->GameThread_GetRenderTargetResource();
    FReadSurfaceDataFlags ReadSurfaceDataFlags;
    RenderTarget->ReadPixels (
        PixelData,
        ReadSurfaceDataFlags,
        FIntRect (0, 0, FrameWidth, FrameHeight));
    
    bWaitingOnPixelData = true;
    bPixelDataReady = true;
}

void ACapturePawn::ReadPixelsAsync() {

//    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("ReadPixelsAsync Begin"));

    FRenderTarget* RenderTarget =
        CaptureComponent->TextureTarget->GameThread_GetRenderTargetResource();
    FIntRect InRect(0, 0, RenderTarget->GetSizeXY().X, RenderTarget->GetSizeXY().Y);
    
    PixelData.Empty(RenderTarget->GetSizeXY().X * RenderTarget->GetSizeXY().Y);
    PixelData.Reset();
    FReadSurfaceDataFlags InFlags(RCM_UNorm, CubeFace_MAX);
    
    // Read the render target surface data back.
    struct FReadSurfaceContext
    {
        FRenderTarget* SrcRenderTarget;
        TArray<FColor>* OutData;
        FIntRect Rect;
        FReadSurfaceDataFlags Flags;
        bool* bFinishedPtr;
    };
    
    FReadSurfaceContext ReadSurfaceContext =
    {
        RenderTarget,
        &PixelData,
        InRect,
        InFlags,
        &bPixelDataReady
    };
    
    ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
        ReadSurfaceCommand,
        FReadSurfaceContext, Context, ReadSurfaceContext,
        {
            RHICmdList.ReadSurfaceData(
                Context.SrcRenderTarget->GetRenderTargetTexture(),
                Context.Rect,
                *Context.OutData,
                Context.Flags
            );
            
            *Context.bFinishedPtr = true;
        }
    );
    
//    GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("ReadPixelsAsync Finish"));
    
    bWaitingOnPixelData = true;
}
