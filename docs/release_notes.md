# EchoPod release notes

## [0.1.0-alpha] - 2026-11-06

### What's Working
- Complete board bring-up for the FiiO Snowsky Echo port: pin mapping (`board_pins.h`), display panel integration (`panel_vop.c`, `panel_port.h`), and image updater patches applied.
- Firmware build successfully compiled, flashed, and verified on hardware.

### Known Issues
- **Audio Cracking / Popping:** Occasional audio crackles or pops in **USB DAC mode**, specifically noticed during Windows system notification sounds.
- **Inverted Channels:** Left and right stereo channels are swapped/reversed (verified via Windows sound check). Spatial audio processing remains functional.