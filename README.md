# unreal-capture
Unreal Capture Functionality Explorer

#### Unreal Engine Version:
```
4.14.3
```

#### Screenshot Capture:
1. Press ```Space``` to capture screenshot.

2. ```Stat FPS``` in command console to show FPS info. 

3. ```RenderTarget->ReadPixels``` and ```ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER``` Performance Test:

Test machine:
```
Macbook Pro
2.5 GHz Intel Core i7
16 GB
NVIDIA GeForce GT 750m
```

Test result:
```
RenderTarget->ReadPixels
FPS: ~24fps, Frame: ~40ms

ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER
FPS: ~30fps, Frame: ~30ms
```

#### Audio Capture:
1. Press ```A``` to start/stop audio capture.

2. If run in editor mode, captured audio will be saved in ```../Epic Games/UE_4.13/Engine/Binaries/WinXX```
