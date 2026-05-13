# SGM Delay

Stereo delay plug-in built with JUCE/CMake.

## Features

- VST3 and standalone builds
- BPM-synced time divisions: 8/1, 4/1, 1/2, 1/4, 1/8, 1/16, 1/8T, 1/8D
- Free millisecond delay mode
- Feedback, spread, stereo width, wet/dry mix
- Ping-pong feedback
- Low cut and high cut tone shaping
- Delay-time modulation
- Ducking
- Freeze mode

## VST Overview

- Format: VST3 and standalone app
- Type: audio effect, not a synth or MIDI effect
- Channel layout: mono or stereo input/output
- Sync: follows host BPM for tempo divisions and resets the delay buffer on transport jumps
- Tail: reports a 3 second tail to the host
- State: all parameters are saved and restored by the host session

## Controls

- Mix: dry/wet balance.
- Time: selects the delay timing mode. Tempo divisions follow the host BPM; `Msec` uses the manual millisecond value.
- Msec: manual delay time from 1 ms to 10000 ms.
- Feedback: amount of delayed signal fed back into the delay line.
- Spread: offsets left and right delay timing for stereo movement.
- Low Cut: removes low frequencies from the repeats.
- High Cut: darkens repeats by rolling off high frequencies.
- Mod Rate: modulation LFO speed.
- Mod Depth: delay-time modulation depth in milliseconds.
- Width: stereo width of the delayed signal.
- Ducking: lowers the wet signal while the dry input is present.
- Ping Pong: alternates feedback across left and right channels.
- Freeze: holds the delay buffer for sustained repeats.

## Build

Requirements:

- CMake 3.22 or later
- Visual Studio 2022 with the Desktop development with C++ workload
- Internet access on the first configure so CMake can fetch JUCE 7.0.12

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

After copying, rescan plug-ins in your DAW and look for `SGM Delay`.

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

The installer places the VST3 plug-in in:

```text
C:\Program Files\Common Files\VST3\SGM Delay.vst3
```

It also installs the standalone app in:

```text
C:\Program Files\Sagami\SGM Delay
```

## License
MIT License
