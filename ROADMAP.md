# EchoPod roadmap

Status: `device` (works on the player) · `sim` (works in the simulator only) · `in progress` · `todo`

## Where it stands

EchoPod features robust support for Chinese, Japanese, and Korean text, M3U and M3U8 playlists, gapless playback, a 7-band equalizer, Bluetooth, USB DAC mode, large-card library management, and synchronized lyrics parser with a two-line Now Playing view and memory optimizations. The simulator setup targets the 480×222 Echo screen layout with centered Cover Flow, and active development is focused on porting and validating these features directly onto the Echo hardware stack. `docs/known-issues.md` lists what's missing or unproven.

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
| Updates and the way back | device | EchoPod installs its own updates and stock firmware via file copy |
| Configurable start screen | sim / device | Settings → Start screen |

## Next (Echo Hardware Porting)

| Item | Status |
|---|---|
| Recovery check (maskrom mode, rkdeveloptool, Zadig driver binding, read-only ld/td and flash backup) | in progress |
| Echo hardware layer integration (screen driver/init, pin table, backlight, jack polarity, DAC setup, storage layout, ECHOVnnn filenames, memory map) | in progress |
| Unverified Echo values check (DAC settings, backlight, jack values vs. stock firmware; testing Filter and Balance settings) | todo |
| Memory and size re-measurement (handling ~11 KB buffer increase for wider screen and tight code area limits via overlays/trimming) | todo |
| First flash and device testing (direct maskrom write, subsequent updates via player updater, testing lyrics and large libraries on device) | todo |
| Visual UI redesign & screen verification (checking all screens at 480×222, applying user look preference like retro cassette/minimal dark/high contrast, portrait lyrics, and toggle) | todo |

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

- Echo-sync on Mac and Linux
- Installing from a Mac
- Testing on more players
- pid.codes USB ID assignment (retaining shared test ID for now)
- Bluetooth stays SBC-only due to hardware limits