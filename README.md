# AETHER

Replacement firmware for the Snowsky (FiiO) Echo (NOT MINI) music player. It has an iPod-style Cover Flow browser, a music library built on the player, album favorites, M3U & M3U8 playlists, an Up Next queue, and synchronized lyrics support. It isn't made or supported by FiiO.

AETHER is a fork of EchoPod by BasilSkyWalk, tailored for the FiiO Snowsky Echo (NOT MINI) music player with numerous optimizations and porting updates.

- **Installing and using it:** `docs/user_guide.md`.
- **Read first:** `docs/known_issues.md`. AETHER is early, and hardware testing is expanding.
- **What's new:** `docs/release_notes.md`.

The hardware knowledge it relies on comes largely from the [ReChord](https://github.com/brnomt/ReChord) project's research (`docs/hardware.md`, `CREDITS.md`).

## What it does

- **Plays** FLAC, ALAC, MP3, AAC, Opus and WAV, from 44.1 to 192 kHz with 24-bit output, gapless.
- **Cover Flow**, with every album and a Random album tile, and **Now Playing** with the cover, progress, seeking, and a two-line synchronized lyrics view.
- **A library built on the player:** Artists, Album Artists, Albums, Genres, Folders, All Songs, Playlists (M3U and M3U8) and Favorites, with fast scroll and letter jumps. Handles large SD cards. Or build it on a computer with **echo-sync**.
- **Queue and modes:** shuffle songs or albums, repeat, and Up Next (Play Next, Add to Up Next). The queue and position come back after power-off.
- **Equalizer:** FiiO's presets and a 7-band Custom EQ.
- **Portrait mode:** Settings → Orientation turns the screen upright either way, with layouts made for it. The volume rocker turns with it.
- **Languages:** Settings → Language: English, Simplified Chinese, Japanese, Korean, German, French, Spanish, Italian, Portuguese, Dutch, Swedish, Danish, Norwegian and Finnish. Chinese, Japanese, and Korean text rendering is fully supported.
- **Audiobooks and podcasts** on their own shelves, with a bookmark for each.
- **Bluetooth headphones:** pairing, up to six remembered, reconnecting by itself.
- **USB:** both drives as mass storage, or the player as a USB DAC.
- **Everyday controls:** pause, volume and skip with the screen off, a sleep timer, auto off and a charge screen.
- **Updates itself** from a file copy, and goes back to FiiO's firmware the same way.

## Build on Windows

Install [MSYS2](https://www.msys2.org) and, in its CLANG64 shell, the packages `mingw-w64-clang-x86_64-{clang,compiler-rt,cmake,ninja,SDL2}` and `mingw-w64-clang-x86_64-arm-none-eabi-{gcc,binutils,newlib}`, plus Python 3 with `pip install -r tools/requirements.txt`. With `C:\msys64\clang64\bin` first on the PATH (and not `C:\msys64\usr\bin`, whose git ignores this repo's line endings):

```sh
CMAKE_GENERATOR=Ninja CC=clang cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

## Build the firmware

```sh
cmake --preset release
cmake --build --preset release
```

This writes `build/release/ECHOV400.IMG`. The `device` preset builds the same image with the developer Diagnostics rows. `docs/device_install.md` covers installing builds during development.

## Use your music library in the simulator

```sh
mkdir -p build/system
./build/dev/platform/sim/echo_sim --card D:/Music --system-dir build/system
```

The simulator behaves like the player: on the first start it creates `library.db`, `covers.bin` and the save files in the system folder, scans the music folder and finds album covers. Settings → Rescan library scans again. The music folder is only ever read.

To build `echo-sync` yourself on Windows: `python -m pip install -r tools/requirements.txt -r tools/requirements-build.txt`, then `python -m PyInstaller --onefile --name echo-sync --paths tools tools/echo_sync.py`.
