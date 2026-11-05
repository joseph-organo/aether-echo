# EchoPod user guide

EchoPod is a replacement firmware for the Snowsky (FiiO) Echo Mini music player. It has a Cover Flow album browser, a music library built on the player, favorites, playlists and an Up Next queue. It is not made or supported by FiiO.

Read `docs/known-issues.md` before you install. EchoPod has been tested on one player so far.

**Use it at your own risk.** EchoPod comes with no warranty (see `LICENSE`). Replacing a player's firmware can leave it unable to start; during development that happened, and the player was brought back through maskrom (`docs/device-recovery.md`). Only install it if you're prepared to do that.

## Which players

- **Echo Mini 8 GB: supported.** Its FiiO update file is called `ECHOVnnn.IMG`.
- **Echo Mini 512 MB: not supported.** Its FiiO update file is called `MINIVnnn.IMG`. Its firmware ignores EchoPod's file, and EchoPod refuses its firmware.

Tested starting points: FiiO firmware 3.7.0, 3.9.0 and 4.0.0 (Settings → About on the player shows yours).

## Before you install

1. Copy anything you want to keep off the player's internal storage.
2. Charge the battery.
3. Download `ECHOV400.IMG` and `SHA256SUMS.txt` from the [EchoPod releases](https://github.com/BasilSkyWalk/echopod/releases), and check the file:
   - Windows (PowerShell): `Get-FileHash ECHOV400.IMG`

   The result must match the line for `ECHOV400.IMG` in `SHA256SUMS.txt`. If it doesn't, download it again.

## Install

1. Connect the player to your computer. The internal storage appears as a drive called ECHO MINI.
2. Copy `ECHOV400.IMG` to the top level of that drive, not into a folder. Delete any other `ECHOVnnn.IMG` file there first: the player installs the one with the highest number.
3. Eject the drive (Windows: Safely Remove), then unplug.
4. Take the microSD card out.
5. Switch the player off and on again. It shows "Updating, Don't power off" for about a minute or two. Leave it alone until EchoPod starts.
6. Put the microSD card back. On the first start with a card, EchoPod builds its library and album covers, which takes a minute or two (about 1 minute 45 seconds for 1,563 tracks and 144 albums).

If "Updating" flashes for under a second and FiiO's firmware comes back, the install didn't happen. See "If something goes wrong" below. One tester whose FiiO firmware kept refusing the file got it to install after first updating to FiiO's 3.8 firmware.

## Your music

- Put music on a microSD card formatted FAT32 or exFAT, in any folder layout.
- EchoPod plays FLAC, ALAC (Apple Lossless `.m4a`), AAC (`.m4a`), Opus (`.opus`), MP3 and WAV, up to 192 kHz. HE-AAC isn't supported yet. It lists the other formats FiiO lists, and says when it can't play one.
- Album covers come from art inside each album's first track, or from a `cover`, `folder`, `front` or `albumart` JPEG or PNG next to the tracks.
- `.m3u` and `.m3u8` playlists anywhere on the card appear under Music → Playlists.
- Libraries of many thousands of tracks work; `docs/known-issues.md` lists the sizes tested so far.
- EchoPod keeps its own files in a hidden `/.echo` folder on the card, about 110 MB of them. Don't delete it unless you want the library rebuilt.

## Copying music over USB

1. Connect the player. Two drives appear: internal storage and the SD card.
2. Copy music to the SD card.
3. **Eject both drives before you unplug.** If you unplug first, recent changes may be lost, and the player warns you.
4. When music changed, the player rebuilds its library by itself. Copying other files doesn't trigger a rebuild. Settings → Rescan library rebuilds it on demand.

## Using the player as a USB DAC

**Untested on a player so far** (see `docs/known-issues.md`). In this mode the player is a sound card for your computer or phone: their audio plays through the player's headphone outputs.

1. On the player, Settings → USB mode → DAC.
2. Plug in the USB cable. The player shows "USB DAC". No driver is needed on Windows 10 and later (phones need a USB-C adapter that passes data).
3. On the computer, choose EchoPod as the sound output (Windows: the speaker icon in the taskbar). The player shows the format, for example "Playing from computer · 48 kHz 24-bit".
4. Use the computer's volume, the player's volume keys, or both. DAC mode has its own volume, separate from music: it starts at 30 of 50, the screen shows it ("48 kHz 24-bit · Volume 30"), holding Vol± changes it quickly, and the player remembers it for next time. Start low.
5. Unplug when you're done, or hold Back for a second and let go to go back to the player's own music (it keeps charging).

The player accepts 16 or 24-bit at 44.1 and 48 kHz; the computer picks one (Windows: Sound settings → EchoPod → Properties → Advanced). Music on the player stops while it's a DAC, and your drives don't appear: set USB mode back to Storage to copy music. The setting takes effect the next time you plug in. A charger works the same in both modes.

## Build the library on a computer

Rebuilding on the player takes a minute or two (1,563 tracks and 144 albums take about 1 minute 45 seconds). A computer can build the same library and album covers instead, straight onto the card, and tell the player that the card has already been scanned.

The tool is called **echo-sync**. Download it from the release page, under Assets; it's one file, with nothing to install:

| Computer | File |
|---|---|
| Windows | `echo-sync-windows-x64.exe` |

**Windows**

1. Copy your music to the card, in the player over USB or in a card reader.
2. Double-click `echo-sync-windows-x64.exe`. Windows may say it "protected your PC" because the file isn't signed: click **More info**, then **Run anyway**.
3. It finds a card the player has used and asks "Card to sync [E:\]": press Enter. For another card, type its drive letter (or drag the drive onto the window) and press Enter.
4. When it says "Done", press Enter to close it, eject the card (in the player: both drives), and put it back in the player or unplug.

From a command prompt, `echo-sync-windows-x64.exe E:` does the same without asking.

You can also use Python if running from source: `tools\echo_sync.bat E:` finds Python for you.

It reads the whole card, as the player does, and writes `library.db`, `covers.bin` and `scan.bin` into the card's `/.echo` folder. If the player has never used the card, it asks before creating `/.echo` (`--yes` creates it without asking). Files the player made are overwritten in place and keep their size; missing ones are made at the player's sizes (4 MB, 80 MB and 4 KB). It then prints what it found, for example "1,402 tracks, 120 albums, 3 playlists" and "115 covers, 5 without art".

**Why the player doesn't rescan.** After a USB session the player checks for changes: it compares the names, sizes and modification times of the music, playlists and cover images on the card with a fingerprint it keeps in `scan.bin`. The computer writes the same fingerprint, so the check finds nothing new and the player keeps the computer's library. With a card reader there's no USB session, so the player uses the new library straight away.

**When the player rescans anyway.** If the fingerprints differ, the player rescans as it would without the computer, and nothing is lost. That happens when:
- the music changed after the computer ran;
- the card is exFAT and a file's time was saved without a time-zone tag (modern Windows always adds one);
- the card has folders more than 31 levels deep or paths longer than 1,023 bytes (the player can't fingerprint those).

To see whether it works on your card, let the player scan it once, then run echo-sync with `--check` without changing anything (for example `echo-sync-windows-x64.exe E: --check`). "They match" means the player won't rescan after the next sync.

**After a firmware update** the player checks the music once when it first starts ("Checking for changes"), and rescans only if the music changed, or if it can't read the library on the card (as after updating to 0.2.0, which changed the library format). `scan.bin` remembers the firmware version; the computer keeps it. A `scan.bin` the computer had to create has no version yet, so the player checks once on its next start too.

| Option | Effect |
|---|---|
| `--dry-run` | Read everything and say what would be written, but write nothing |
| `--no-covers` | Build the library only; `covers.bin` stays as it is, so new albums show without covers until the next rescan on the player |
| `--covers-only` | Rebuild `covers.bin` from the card's `library.db` |
| `--no-fingerprint` | Leave `scan.bin` alone, so the player compares the music with its own last scan |
| `--check` | Only compare the computer's fingerprint with the player's |
| `--yes` | Create `/.echo` without asking |
| `--verbose` | List the albums without art |