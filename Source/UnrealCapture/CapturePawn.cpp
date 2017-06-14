// Fill out your copyright notice in the Description page of Project Settings.

#include "UnrealCapture.h"
#include "CapturePawn.h"
#include "ImageUtils.h"
#include "Misc/FileHelper.h"
#include "HighResScreenshot.h"

#include <thread>

#if PLATFORM_WINDOWS
// AudioCaptureLib DLL
typedef void(*_getInitialize)(); // Declare a method to store the DLL method initialize.
typedef void(*_getAudioEncoding)(); // Declare a method to store the DLL method audioEncoding.
typedef void(*_getStopEncoding)(); // Declare a method to store the DLL method stopEncoding.
typedef void(*_getReleaseLibResources)(); // Declare a method to store the DLL method releaseLibResources.

_getInitialize m_getInitializeFromDll;
_getAudioEncoding m_getAudioEncodingFromDll;
_getStopEncoding m_getStopEncodingFromDll;
_getReleaseLibResources m_getReleaseLibResourcesFromDll;

void *v_dllHandle;
#endif

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
    
#if PLATFORM_WINDOWS
    // Audio capture vars
    bAudioCaptureStart = false;
    bAudioCaptureNeedStop = false;
#endif
}

// Called when the game starts or when spawned
void ACapturePawn::BeginPlay()
{
    Super::BeginPlay();
    
    // Create directory
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    FString Dir = GetGameDir();
    // Directory Exists?
    if (!PlatformFile.DirectoryExists(*Dir))
    {
        PlatformFile.CreateDirectory(*Dir);
        if (!PlatformFile.DirectoryExists(*Dir))
        {
            GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Create directory failed"));
        }
    }
    
#if PLATFORM_WINDOWS
    // Import DLL
    if (ImportDLL("AudioCapturePlugin", "AudioCaptureLib.dll"))
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Load dll success"));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Load dll fail"));
    }
    
    // Import DLL methods
    if (ImportMethodInitialize())
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Import method initialize success"));
    }
    else {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Import method initialize fail"));
    }
    if (ImportMethodAudioEncoding())
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Import method audioEncoding success"));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Import method audioEncoding fail"));
    }
    if (ImportMethodStopEncoding())
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Import method stopEncoding success"));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Import method stopEncoding fail"));
    }
    if (ImportMethodReleaseLibResources())
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Import method releaseLibResources success"));
    }
    else
    {
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("Import method releaseLibResources fail"));
    }
    
    m_getInitializeFromDll();
#endif
}

// Called every frame
void ACapturePawn::Tick( float DeltaTime )
{
    Super::Tick( DeltaTime );
    
#if PLATFORM_WINDOWS
    // Audio capture
    if (bAudioCaptureStart)
    {
        m_getAudioEncodingFromDll();
    }
    if (bAudioCaptureNeedStop)
    {
        m_getStopEncodingFromDll();
        bAudioCaptureNeedStop = false;
    }
#endif
    
    if (ReadPixelsTimeWaited < 1.0f) {
        ReadPixelsTimeWaited += DeltaTime;
        return;
    }
    
    // Screenshot
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
    
#if PLATFORM_WINDOWS
    InputComponent->BindAction(
                               "AudioCapture",
                               IE_Pressed, this,
                               &ACapturePawn::AudioCapture);
#endif
}

// Called in several places to guarantee the life of the Actor is coming to an end
void ACapturePawn::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
#if PLATFORM_WINDOWS
    FreeDLL();
#endif
}

FString ACapturePawn::GetGameDir()
{
    FString Path = FPaths::ConvertRelativePathToFull(FPaths::GameDir());
    return Path + "Capture/";
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
    // Save file path
    FString Filename = GetGameDir() + "Screenshot.png";
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

#if PLATFORM_WINDOWS
// Method to import a DLL
bool ACapturePawn::ImportDLL(FString folder, FString name)
{
    FString filePath = *FPaths::GamePluginsDir() + folder + "/" + name;
    
    if (FPaths::FileExists(filePath))
    {
        v_dllHandle = FPlatformProcess::GetDllHandle(*filePath); // Retrieve the DLL.
        if (v_dllHandle != NULL)
        {
            return true;
        }
    }
    return false;	// Return an error.
}

// Imports the method initialize from the DLL.
bool ACapturePawn::ImportMethodInitialize()
{
    if (v_dllHandle != NULL)
    {
        m_getInitializeFromDll = NULL;
        FString procName = "initialize";	// Needs to be the exact name of the DLL method.
        m_getInitializeFromDll = (_getInitialize)FPlatformProcess::GetDllExport(v_dllHandle, *procName);
        if (m_getInitializeFromDll != NULL)
        {
            return true;
        }
    }
    return false;	// Return an error.
}

// Imports the method audioEncoding from the DLL.
bool ACapturePawn::ImportMethodAudioEncoding()
{
    if (v_dllHandle != NULL)
    {
        m_getAudioEncodingFromDll = NULL;
        FString procName = "audioEncoding";	// Needs to be the exact name of the DLL method.
        m_getAudioEncodingFromDll = (_getAudioEncoding)FPlatformProcess::GetDllExport(v_dllHandle, *procName);
        if (m_getAudioEncodingFromDll != NULL)
        {
            return true;
        }
    }
    return false;	// Return an error.
}

// Imports the method stopEncoding from the DLL.
bool ACapturePawn::ImportMethodStopEncoding()
{
    if (v_dllHandle != NULL)
    {
        m_getStopEncodingFromDll = NULL;
        FString procName = "stopEncoding";	// Needs to be the exact name of the DLL method.
        m_getStopEncodingFromDll = (_getStopEncoding)FPlatformProcess::GetDllExport(v_dllHandle, *procName);
        if (m_getStopEncodingFromDll != NULL)
        {
            return true;
        }
    }
    return false;	// Return an error.
}

// Imports the method releaseLibResources from the DLL.
bool ACapturePawn::ImportMethodReleaseLibResources()
{
    if (v_dllHandle != NULL)
    {
        m_getReleaseLibResourcesFromDll = NULL;
        FString procName = "releaseLibResources";	// Needs to be the exact name of the DLL method.
        m_getReleaseLibResourcesFromDll = (_getReleaseLibResources)FPlatformProcess::GetDllExport(v_dllHandle, *procName);
        if (m_getReleaseLibResourcesFromDll != NULL)
        {
            return true;
        }
    }
    return false;	// Return an error.
}

// If you love something  set it free.
void ACapturePawn::FreeDLL()
{
    m_getReleaseLibResourcesFromDll();
    
    if (v_dllHandle != NULL)
    {
        m_getInitializeFromDll = NULL;
        m_getAudioEncodingFromDll = NULL;
        m_getStopEncodingFromDll = NULL;
        m_getReleaseLibResourcesFromDll = NULL;
        
        FPlatformProcess::FreeDllHandle(v_dllHandle);
        v_dllHandle = NULL;
    }
}

void ACapturePawn::AudioCapture()
{
    if (bAudioCaptureStart)
    {
        // Stop audio capture
        bAudioCaptureStart = false;
        bAudioCaptureNeedStop = true;
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Stop audio encoding"));
    }
    else
    {
        // Start audio capture
        bAudioCaptureStart = true;
        bAudioCaptureNeedStop = false;
        GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Start audio encoding"));
    }
}
#endif
