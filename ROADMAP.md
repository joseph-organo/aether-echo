# AETHER roadmap

Status: `device` (works on the player) · `sim` (works in the simulator only) · `in progress` · `todo`

## Current Status (v0.1.0-alpha)
- **v0.1.0-alpha Release:** Initial hardware bring-up and flashing achieved.
- **Active Testing:** Initial hardware playback testing underway; investigating USB DAC audio cracking and stereo channel inversion.

## Next (Echo Hardware Porting & Bug Fixes)
| Item | Status |
|---|---|
| Investigate and resolve USB DAC audio cracking / notification buffer glitch | in progress |
| Diagnose and correct left/right stereo channel swap in the audio pipeline | in progress |
| Full hardware stability and audio validation | todo |

## Features

| Feature | Status | Notes |
|---|---|---|
| FLAC, MP3 and WAV playback | device | Our own FLAC decoder on the second core with the hardware LPC accelerator; libmad for MP3; 24-bit output at 44.1–192 kHz |
| Gapless playback and fades | device | DAC fades around play, pause, seek, skip and rate changes; MP3s trimmed by their LAME tag's encoder delay and padding |
| Resume after power-off | device | The exact queue and position come back |
| Library built on the player | device | At first start, after music changes over USB, and from Settings → Rescan library; handles large cards efficiently |
| Album covers built on the player | device | Embedded art or `cover`/`folder`/`front`/`albumart` JPEG or PNG; optimized for player RAM limits |
| Browse: Artists, Album Artists, Albums, Genres, Folders, All Songs, Playlists, Favorites | device | Fast scroll with letter jumps in A–Z lists |
| Cover Flow | device | Every music album, by album artist; a Random album tile first; centered for 480×222 screens |
| Now Playing & Lyrics | device | Cover, progress, seek, modes, volume, and a two-line synchronized lyrics view (`.lrc` parser with memory fixes) |
| Queue, shuffle, repeat, Up Next | device | Shuffle Songs or Albums over lists of any size; Play Next and Add to Up Next |
| M3U and M3U8 playlists | device | Full support for external playlists |
| Album favorites | device | Hold Play on an album |
| Audiobooks and podcasts | device | Dedicated folders on Home, kept out of Music; bookmarking and skip controls |
| Library and covers built on a computer | device | echo-sync support for fast pre-indexing |
| Screen-off controls | device | Pause, volume and track skip with the screen off |
| Portrait mode | device | Settings → Orientation: Portrait or Portrait flipped, with dedicated layouts |
| Languages | installed once | Multi-language UI support with full CJK text rendering |
| Settings | device | Shuffle, repeat, equalizer, sleep timer, screen timeout, auto off, brightness, orientation, gain, output, start screen, USB mode |
| Battery and power | device | Battery level, charging, charge screen, low-battery shutdown, auto off |
| Bluetooth headphones | device | Pair, remember up to six, auto-reconnect, switch and disconnect; headphone controls |
| USB mass storage | device | Internal storage and high-speed SD card read/write |
| USB DAC mode | device | USB Audio Class 1.0 sound card support |
| Updates and the way back | device | AETHER installs its own updates and stock firmware via file copy |
| Configurable start screen | sim / device | Settings → Start screen |

## Roadmap Features

| Item | Status |
|---|---|
| Refuse to start on an almost empty battery | todo |
| ReplayGain | todo |
| Browsing while covers build | todo |
| Faster cover decoding | todo |
| Parametric EQ (free frequency and width per band) | todo |
| Playback speed for audiobooks and podcasts | todo |
| APE playback | todo |
| HE-AAC playback | todo |
| Tolerant filename matching for lyrics | todo |

## Later / Skipped on Purpose

- Installing from a Mac
- Testing on more players
- pid.codes USB ID assignment (retaining shared test ID for now)
- Bluetooth stays SBC-only due to hardware limits