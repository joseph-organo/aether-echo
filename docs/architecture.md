# Architecture

AETHER, replacement firmware for the Snowsky Echo (full-size model, NOT Mini). The app layer is portable C. It runs in a desktop simulator (Windows) during development and on the device on top of a thin hardware layer.

## Layers

```
platform/sim        platform/device
        \                  /
         implement  hal  (interfaces only)
                     |
                    app          composition root: theme, screens, player, stores, main step
           /     /       \        \
         ui   fonts    settings   hal
           \   /           |
            gfx        playback
              \            |
               \        library
                \         |    \
                 \        |   image
                   \      |    /
                      core
```

| Layer | Path | May depend on | Purpose |
|---|---|---|---|
| core | `src/core` | nothing | Pure logic: deadlines, key gestures, frame stats, UTF-8, string building, formatting, hashing, seeded random, the power policy |
| gfx | `src/gfx` | core | RGB565 colors and blending, rectangles, clipped canvases, strip rendering, 4-bit anti-aliased text, images drawn into perspective quads |
| fonts | `src/fonts` | gfx | Generated glyph tables (see Generated code) |
| ui | `src/ui` | core, gfx | Screen interface and stack, navigation intents, list view, menus, choice screens, messages, toasts, header and status icons, labels, marquee, fades, Cover Flow layout and motion, theme type |
| image | `src/image` | core | Cover decoding for the rescan: a bump arena, a buffered reader, the 112 px area-average grid, inflate, PNG and JPEG (see Covers on the device) |
| library | `src/library` | core, image | Byte sources, a 4×512 B block cache, the library database reader, browse queries, the favorites set and the covers file reader (see `docs/library-format.md`) |
| codec | `src/codec` | core | FLAC decoding (up to 2 channels, 24 bits, 4,608-sample blocks) from a read callback, with header and frame CRC checks, and seeking by bisection on frame headers; ALAC decoding (up to 2 channels, 16/20/24 bits, frames up to 4,608 samples); and an MP4 reader that finds the sound track and turns its sample tables into a stream of length-prefixed records (see Device playback) |
| playback | `src/playback` | core, library | Play queue (list order, songs shuffle, album shuffle, repeat, gapless look-ahead) and the resume file format |
| settings | `src/settings` | core, playback | The settings values, their defaults and option tables, and the settings file format |
| dsp | `src/dsp` | nothing | The equalizer: fixed-point peaking filters, their design for a sample rate, and the automatic preamp (see Equalizer) |
| sound | `src/sound` | core, dsp | The equalizer presets, the Custom bands and the `equalizer.bin` format |
| hal | `src/hal` | core, dsp (types only) | Hardware interfaces: display (with power and brightness), input, clock, files, audio (with gain and output), power (battery, power off), system (firmware tag, CPU clock), Bluetooth (module power, scan, pair, disconnect, status) |
| app | `src/app` | core, gfx, fonts, ui, library, playback, settings, sound, hal | Default theme, screen tree, library, settings and favorites stores, cover cache, player controller, browse, Cover Flow, Settings and Now Playing screens, frame overlay, the main step |
| platform | `platform/*` | app, hal, codec, the OS or SDL | Implements hal and owns `main()` |

The CMake targets enforce these rules. Each module exposes only its own `include/<module>/` directory, and a module can only include headers from targets it links. Anything in a module's own directory, outside `include/`, is private.

## Screens and input

- A `Screen` is a title plus callbacks: `enter`, `handle_key`, `update` and `draw`. `handle_key` returns a `ScreenAction` (none, push, pop, or pop to root), and the app applies it to the `ScreenStack`. Screens never reach into the stack themselves.
- Raw key transitions come from `hal`. `core/key_tracker` turns them into gestures (press, long press, repeat), and `ui/navigation` maps gestures to intents (previous, next, select, back, home). Remapping buttons is a change in one place.
- Each key has a gesture type: tap-or-hold, repeat (press on down, then repeats), or tap-hold-repeat (tap on release, long press, then repeats). The volume keys repeat, except on screens that set `hold_volume_keys` (Now Playing), where they're tap-hold-repeat so a hold can skip tracks. The app switches the type whenever the top screen changes.
- Screen instances are static and owned by the app's `AppScreens`. There is no heap.
- A long Play is `NAVIGATION_OPTIONS`. Lists hand it to their `hold` action; in browse lists it toggles an album's favorite.
- Smart Play: in a browse list, ▶‖ on the track the player already has loaded (playing or paused) pushes Now Playing without restarting it. ≡ pushes Now Playing from any screen while the player has a track, and does nothing on Now Playing itself.
- Lists draw the selected row as a highlight line over a gradient, a › chevron on rows that open another screen (`ListModel.opens`), and a slim inset scrollbar thumb. Menu items can be hidden by a `shown` callback, and a menu can give the right of its content to a side panel (`MenuPanel`). Home uses that for `home_art`: the playing album's cover with a reflection, or library covers cycling every 5 s when nothing is loaded; Home lists Now Playing last and only while the player has a track.
- Settings is a menu whose rows show their current value. Every row opens the same `ChoiceScreen`, re-initialized from a table of `SettingChoice` entries (title, options, current, choose), and the current option gets a ✓.

## Power

- `core/power_policy` decides what keys mean for power. A tap of Power turns the screen off or on, and a long Power asks for shutdown. With the screen off (after a tap or a timeout), ▶‖ and vol± are background keys: the app maps them as on Now Playing (tap vol± for volume, hold vol± to skip, ▶‖ to pause) and runs them without waking the screen. ≡ and ↩ do nothing. While the screen is off, the volume keys use tap-hold-repeat on every screen, so a hold skips. The policy also owns the screen timeout and the sleep timer.
- The app runs each key event through the policy before the screen sees it. When the policy turns the display on or off, the app calls `hal_display_set_power` and stops rendering while the screen is off. Settings are saved each time the screen goes off.
- Shutdown (long Power, or the sleep timer running out) saves the resume file and settings, turns the screen off and calls `hal_power_off()`. The simulator then quits; the device cuts power.
- Battery level (`device/battery_gauge`): the battery is read once a second, and the percent comes from the average of the last 64 readings (about a minute) through RECHO's 11-point curve (3450 mV = 0 %, 4200 mV = 100 %; stock shows only 5 bars, full above 3830 mV), so short dips under load (a song starting, card reads, the backlight) don't show. While charging it reads 50 mV lower and never shows more than 99 %, since the charging current lifts the terminal voltage most near full (stock shows full on the cable only at 4250 mV or once the charger reports done). The average restarts from the latest reading when USB power comes or goes, so the charger's lift doesn't linger after unplugging. The low-battery check and Diagnostics → Battery use the last 8 readings. The shown percent follows the power source: on USB power it rises with any 2-point change but only falls by 10 points or more, on battery the other way round.
- Low battery (`core/low_battery`, stock's rule): every 250 ms the app reads the battery, and if it stays below 3450 mV for 2 s without USB power, it shuts down the same way with "Battery low" in place of "Goodbye".
- CPU clock (stock's rule: the clock follows the work, never the screen): 200 MHz while the screen is on or audio is up, 96 MHz (stock's floor) only with the screen off and `HalAudioStatus.idle` true (output and core 1 down, 3 s after playback stops). The device's audio HAL raises the clock to 200 MHz before it starts core 1, so a screen change never parks core 1. A USB session runs at 96 MHz at least.
- Waking draws the first frame before the backlight comes on (stock waits for 4 frames), and on the device the backlight then fades in over 200 ms (`device/backlight` ramp, from 1 % to the level's duty on the PWM, advanced by the main loop).
- With the screen off, the device's main loop runs WFI once per pass, as stock does; SysTick (1 ms) wakes it, and the audio ring holds over 80 ms.
- Auto off (Settings, stock's options: Off, 1, 5, 10 min, default 5): `core/power_policy` counts from the last key press; the app holds it off each step while a track plays or USB power is present.
- Brightness 1–10 maps to a backlight duty of 8, 12, 17, 23, 30, 39, 50, 63, 80 and 100 % (`device/backlight`). Below 100 % GPIO2.4 runs stock's 10 kHz PWM, whose period follows PCLK and is reprogrammed after every clock switch. `PWM_CTRL` `0x10003`: continuous, duty polarity negative.

## Settings and favorites

- `app/settings_store` loads `/.echo/settings.bin` (defaults if it's missing or invalid), applies brightness, gain and output through the HAL, and only writes when the encoded bytes changed.
- `hal_dir_open`, `hal_dir_next` and `hal_dir_close` list a folder: name (UTF-8, up to 767 bytes), folder or file, size, and a hidden flag. `.` and `..` are never listed. On the device, `fat_open_directory` and `fat_next_entry` walk FAT16/FAT32 entries and exFAT entry sets; up to 16 folders can be open at once.
- Folder entries are read in runs of up to 8 sectors, like file data, and writes of whole sectors go out as one multi-block write without reading them first.
- On the device, `hal_file_write` overwrites an existing file in place from offset 0 (`fat_write`, FAT16, FAT32 and exFAT) and fails if the file is missing or too short; it never allocates clusters or changes a directory entry.
- The player makes its own files. On every `app_start`, before the first frame, `app/card_files` calls `hal_file_reserve` for `/.echo/settings.bin`, `resume.bin` and `favorites.bin` (4 KB each), `library.db` (4 MB) `covers.bin` (80 MB) and `scan.bin` (4 KB, the music fingerprint), in that order.
- On the device, `fat_reserve` creates missing folders and files as contiguous cluster runs.
- At start-up the saved volume, shuffle and repeat go to the player first, and then a valid resume file overrides them.
- The start screen setting pushes the matching path at start-up.
- `app/favorites_store` loads and saves `/.echo/favorites.bin` and toggles albums by key.

## Languages

- **Catalogue.** Every user-visible English text is a `STRING(STR_NAME, "English")` line in `src/core/include/core/strings_*.def`, which `core/strings.h` turns into the `StringId` enum.
- **Packs.** `assets/lang/languages.json` lists the languages. `tools/lang_pack.py` builds `src/lang/packs/<code>.bin` for every language.
- **Loading.** `app_start` asks `hal_language_pack` for the English pack, then for the saved language once `settings_store_load` has run.
- **Device.** Language packs sit in the image beside the glyph faces. `board/language.c` reads the pack into `.language_ram` (`0x0308F000`, 12 KB).
- **Simulator and tests.** `platform/sim/sim_language.c` reads the packs from `src/lang/packs`.

## Audiobooks and podcasts

- `playback/spoken` decides what is spoken word from the path alone: a track whose first folder is `Audiobooks`/`Audiobook` or `Podcasts`/`Podcast`.
- Bookmarks are keyed by FNV-1a hashes of paths in `/.echo/bookmarks.bin` (4 KB).
- Spoken tracks support skip/jump controls (+30 s / −15 s) and persistent position tracking.

## Equalizer

- **Presets.** Off, Retro, Bass, Heavy, Pop, Jazz, Unique and Custom. Custom has 7 bands (63, 160, 400 Hz, 1, 2.5, 6.3 and 16 kHz, Q 1.0, ±12 dB in 1 dB steps).
- **Filters** (`dsp/equalizer`). One bell filter per non-flat band using a trapezoidal state-variable filter structure.
- **Where it runs.** FLAC/ALAC use Core 1 equalizer shares; MP3, AAC, Opus, and WAV run on Core 0 within `device/audio_engine`.
- **Saving.** Stored in `/.echo/equalizer.bin`.

## Library rescan on the device

- Rescan flags check card fingerprints (`/.echo/scan.bin`) comparing FNV-1a hashes of file attributes across FAT/exFAT structures.
- Overlay architecture: `echo_rescan_overlay` is loaded into HIGHRAM0 (`0x0307A000`) at rescan runtime to preserve the 157 KB Core 0 primary execution window.

## Covers on the device

- Covers decode into `/.echo/covers.bin` as RGB565 bitmaps (112×112 full, 56×56 half).
- Decoders support PNG (all standard color types and depths) and JPEG (baseline/progressive, area-averaged output).

## Cover Flow

- Centered on the 480×222 panel layout. Center cover is 112×112 px; side trapezoids fade and perspective-scale smoothly.
- Position 0 features a "Random album" tile; music albums occupy indices 1 and upward.

## Rendering

- Panel driven directly via RKnano VOP controller (ST7796S, 480×222 visible inside 320×480 frame memory).
- Strip renderer draws 480-wide bands, hashing row outputs (`gfx/frame_diff`) to send only dirty rows over the hardware bus, saving ~85 KB RAM compared to a full framebuffer.

## Device platform

- `platform/device/board`: Handles GPIO, pin muxing, clocks, and peripheral drivers for the FiiO Snowsky Echo hardware layout.
- Audio mute: `GPIO2.1`. Power pair: `GPIO0.14` and `GPIO2.0`. Balanced 4.4 mm jack sense: `GPIO2.8` (high = plugged). Single-ended 3.5 mm jack sense: `GPIO0.13`.
- Firmware update container: `ECHOV400.IMG` generated via `tools/make_image.py`.