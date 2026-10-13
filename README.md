# Excitebike

An OpenGL remake of Excitebike. macOS and Linux build with SDL2. The Visual Studio solution on Windows still uses Win32 and XAudio2.

## Play

Run the game from the repository root so it can find `assets/`.

```bash
make
./build/macos/excitebike
```

On Linux the binary is `build/linux/excitebike`. On Windows it is `build/windows/excitebike.exe`.

| Key | Action |
| --- | --- |
| Left / Right | Move the track selector |
| Enter | Confirm the menu selection |
| Up / Down | Turn |
| Left / Right | Tilt |
| Z | High gear |
| X | Low gear |

## Build

Install a C compiler, GNU Make, and the libraries for your system, then run `make` from the repository root. `make DEBUG=1` builds without optimizations. `make clean` removes `build/`.

### macOS

```bash
brew install sdl2 sdl2_image sdl2_mixer sdl3 pkg-config
make
```

`sdl3` is required when Homebrew's `sdl2` package is the SDL2 compatibility layer.

### Linux

```bash
sudo apt-get install build-essential pkg-config \
  libgl1-mesa-dev libglu1-mesa-dev \
  libsdl2-dev libsdl2-image-dev libsdl2-mixer-dev
make
```

### Windows

Open `excitebike.sln` in Visual Studio. The Win32 libraries live in `lib/`.

To build with SDL2 from MSYS2 UCRT64 instead:

```bash
pacman -S make mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-pkgconf \
  mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_image \
  mingw-w64-ucrt-x86_64-SDL2_mixer
make USE_SDL=1
```

`make windows` from macOS or Linux cross-compiles the Win32 build with `x86_64-w64-mingw32-gcc`.

## Layout

```
assets/            images, sounds, and level data used at runtime
assets/source/     original art
include/game/      game headers
include/framework/ window, input, sound, and OpenGL headers
src/game/          game code
src/framework/     platform code
lib/               Windows libraries used by the Visual Studio build
msvc/              Visual Studio projects
```

## Releases

Pushing a `v*` tag builds Windows, Linux, and macOS archives and publishes them as a GitHub Release. Each archive contains the executable, its SDL libraries, and `assets/`. Extract it and start the game from that folder.

```bash
git tag v1.0.0
git push origin v1.0.0
```

The workflow can also be started by hand from the Actions tab. A manual run uploads build artifacts and does not publish a release.
