# MPC3K Delay

A recreation of the 3-tap delay built into the Akai MPC3000, as a VST3 / Audio Unit plugin for Windows and Mac.

The MPC3000's delay has three echoes ("taps") that all share one memory. Feedback from each tap goes back into that shared memory, so the echoes bleed into each other. That gives it a different feel from a typical multi-tap delay. This plugin copies that behavior, using the same controls and ranges as the original.

## Download

Go to the **[Releases](../../releases)** page and download the zip for your computer:

- `MPC3K-Delay-Windows.zip`
- `MPC3K-Delay-Mac.zip`

## Install

### Windows

1. Unzip the download.
2. Copy the **`MPC3K Delay.vst3`** folder into `C:\Program Files\Common Files\VST3` or wherever you store your VSTs for scanning
3. Open your DAW and rescan plugins.

Nothing else to install. If Windows shows "Windows protected your PC" when opening the standalone app, click **More info → Run anyway**.

### Mac (Apple Silicon and Intel, macOS 11+)

1. Unzip the download.
2. Double-click **`Install.command`**. If macOS won't open it, right-click it, choose **Open**, then click **Open** again.
3. Restart your DAW.

Logic and GarageBand use the Audio Unit version. Ableton, Bitwig, Reaper, FL Studio and most others can use the VST3.

The plugin isn't signed with a paid Apple developer account, so macOS blocks it by default. The installer tells macOS to trust it.

### Try it without a DAW

Both zips include a standalone app, `MPC3K Delay.exe` on Windows and `MPC3K Delay.app` on Mac. Open it, pick your audio device in its settings, and turn on an input.

## Controls

Each of the three taps has:

| Control | Range | What it does |
| --- | --- | --- |
| Volume | 0 to 100 | How loud this echo is |
| Pan | 50L to 50R | Where this echo sits left to right |
| Time | 1 to 1486 ms | How long after the sound this echo plays |
| Feedback | 0 to 100% | How much of this echo is sent back in to repeat |

Plus **Mix** (dry/wet). Leave it at 100% when using the plugin on a send or aux.

## Quick start

- Tap 1 starts on. Set its **Time** to about 375 ms and **Feedback** to about 30%.
- Turn up **Volume** on Tap 2 and Tap 3 to add echoes at other times.
- A tap with Volume at 0 still feeds back, just like the original. You won't hear that tap directly, but you'll hear its repeats through the other taps.

**Careful:** if the three Feedback knobs add up to more than 100%, the echoes keep growing forever. That matches the real MPC3000. Keep your speakers low when experimenting.

## Build from source

Requires CMake 3.22+ and a C++17 compiler (Visual Studio 2022 on Windows, Xcode on Mac). JUCE is downloaded automatically.

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
ctest --test-dir build -C Release
```

The built plugins end up in `build/Mpc3kDelay_artefacts/Release/`.

## Status

This is an early version. The delay's behavior matches the MPC3000 manual. The original's sound character (its grit, hiss, and how the repeats age) still needs to be tuned against recordings of a real unit.
