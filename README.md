# SGM Delay

Stereo delay plug-in built with JUCE/CMake.

## Features

- VST3 and standalone builds
- BPM-synced time divisions: 4/4, 3/4, 1/2, 1/4, 1/8, 1/16, 1/8T, 1/8D
- Feedback, spread, stereo width, wet/dry mix
- Ping-pong feedback
- Low cut and high cut tone shaping
- Delay-time modulation
- Ducking
- Freeze mode

## Build

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

The local build output is:

```text
build/SGMDelay_artefacts/Release/VST3/SGM Delay.vst3
build/SGMDelay_artefacts/Release/Standalone/SGM Delay.exe
```

To install the plug-in for most Windows hosts, copy the `.vst3` bundle to:

```text
C:\Program Files\Common Files\VST3
```

## Installer

This project includes a WiX v4 installer definition. Install the WiX CLI once:

```powershell
dotnet tool install --global wix
```

Then build the plug-in and create the MSI:

```powershell
cmake --build build --config Release
.\Installer\build-installer.ps1 -Configuration Release -ProductVersion 0.1.0 -AcceptEula
```

If WiX is available when CMake configures the project, you can also run:

```powershell
cmake --build build --config Release --target installer
```

The MSI is written to:

```text
build/installer/SGM-Delay-0.1.0-win64.msi
```

## License
MIT License
