# AETHER release notes

## [ECHOV410] - 2026-11-11

AETHER is the new name for this fork of EchoPod for the FiiO Snowsky Echo.

### New
- **Rebrand:** the splash, home menu, USB device name and image are now called AETHER.
- **Audiobooks and podcasts at low sample rates** (22.05 kHz and others) now play. Files at 8, 11.025, 12, 16, 22.05, 24 and 32 kHz are played at a whole multiple of their rate.
- **New menu order:** Music, Audiobooks, Podcasts, Now Playing, Folders, Settings. Folders moved out of Music, and Diagnostics moved into Settings.
- **Flat look:** no gradients, dividers or reflections, and Cover Flow shows five covers.
- **Status bar:** shows repeat (`1` or `ALL`) and shuffle (`S` for songs, `A` for albums) on every screen while they are on.
- **USB DAC mode** turns the backlight off after your Screen timeout and wakes on any key.

### Fixed
- **Left and right swapped** in USB DAC mode (the audio clock could restart one word out of step).
- Late starts of sounds in USB DAC mode.

### Known issues
- A short crackle can still be heard when a new sound starts in USB DAC mode after a pause (mostly noticed with Windows' sound test).
- Bluetooth sound quality on the Echo is poor. It has not been investigated.

## [0.1.0-alpha] - 2026-11-06

First build for the Echo: board pins, display, image updater and USB DAC bring-up, flashed and checked on a player.
