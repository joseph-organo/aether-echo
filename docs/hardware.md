# Echo hardware map

A map of the hardware driven by the FiiO Snowsky Echo (full-size model, NOT Mini) port of EchoPod. Most of the underlying RKnano hardware knowledge was researched by the [ReChord](https://github.com/brnomt/ReChord) project and verified on hardware.

## Chip and Memory

- **SoC:** Rockchip RKnano dual-core Cortex-M3.
  - **Core 0:** Runs the main AETHER firmware, application layer, UI, and event loop.
  - **Core 1:** Dedicated audio decoder engine (FLAC, ALAC, MP3, AAC, Opus) loaded on demand.
- **Core 0 Data RAM:** `0x03004000`–`0x0304C000`. Code execution starts at `0x03050000` (~157 KB code window limit).
- **High RAM:** `0x0307A000`–`0x0309EFFF` (148 KB). Used for housing the library rescan overlay code during card indexing.
- **Core 1 RAM:** `0x01000000`–`0x0105FFFF` (384 KB). Serves as decoder working memory and as scratch memory during library scanning while playback is stopped.
- **Clocks:** Boot ROM starts cores at 24 MHz. AETHER switches PLL to 96 MHz or 200 MHz dynamically depending on load (200 MHz active for audio decoding and UI, 96 MHz floor when idle).

## Display and Keys (Echo Full-Size Specifics)

- **Panel:** Sitronix ST7796S panel, 480×222 visible resolution inside a 320×480 frame memory (49 pixels in from the edge).
  - Driven via the RKnano's hardware LCD controller (VOP, base address `0x60070000`, `device/vop.h`).
  - Muxed GPIO0 pins 15 through 25 handle LCD data/control lines; panel reset is assigned to `GPIO2.15`.
  - Memory access control byte is `0x28` for landscape orientation.
  - Init sequence is a 24-step table starting at `0x591FC` (byte-identical to FiiO `ECHOV200.IMG`). Single panel init sequence with no hardware panel ID check required.
- **Backlight:** 10 kHz PWM on `GPIO2.4` with negative duty polarity (`PWM_CTRL` `0x10003`).
- **Keys:** ▶‖, ≡, ↩, and the volume rocker sit on a resistor ladder read via ADC. Power button is a dedicated digital input.

## Storage

- **Internal Storage (8 GB eMMC):** Driven over DW-MMC controller. Partition layout from block 0:
  1. ID Block (IDB)
  2. Firmware Slot 1 (rockusb sector 0, holds first 5 MiB of firmware image)
  3. Firmware Slot 2 (rockusb sector 69632 / `0x11000`, holds backup firmware slot)
  4. SAVE block
  5. FAT32 user partition (appears as "ECHO" over USB)
  *Note:* AETHER strictly fences writes to the user partition and firmware slots, never touching the ID block or SAVE block.
- **MicroSD Slot:** Card controller supporting FAT16, FAT32, and exFAT with hardware card detect pin.

## Audio Pipeline

- **DAC:** Cirrus Logic CS43131 configured over `I2C0`, fed PCM audio via `I2S1` through DMA channel 5 from a ring buffer in data RAM.
- **Outputs & Routing:**
  - 3.5 mm single-ended and 4.4 mm balanced outputs.
  - **Audio Mute:** `GPIO2.1` (differs from the Mini's `GPIO2.0`).
  - **Power Pair:** `GPIO0.14` and `GPIO2.0` form a power pair held high from boot until power-off.
  - **Jack Detection:** Read via `GPIO0.13` (3.5 mm) and `GPIO2.8` (4.4 mm). The 4.4 mm jack reads as plugged when its sense pin is HIGH.
  - **No DAC Reset Pin:** The full-size Echo board omits the Mini's dedicated hardware DAC reset pin.
- **Hardware Acceleration:** Hardware LPC accelerator alongside Core 1 speeds up FLAC decoding for subframes of order 3 and higher.

## USB

- **Controller:** DesignWare USB 2.0 OTG controller.
- **Storage Mode:** Serves internal storage and SD card as dual LUN Mass Storage Class devices (`1209:0001`).
- **USB DAC Mode:** USB Audio Class 1.0 sound card (`1209:0002`), full-speed only, 16/24-bit stereo PCM at 44.1 kHz and 48 kHz. Synchronized via adaptive ring-level control in `device/stream_drift.c`.
- **Maskrom Mode:** Boot ROM recovery mode exposes USB ID `2207:262d` when powered up with buttons held (`docs/device_recovery.md`).

## Bluetooth

- **Hardware:** External Bluetooth module driven over `UART1` at 115,200 baud (8N1) with short ASCII command strings. Power control on `GPIO2.7`.
- **Audio Routing:** While streaming to Bluetooth headphones, audio routing forces `GPIO2.0` high and `GPIO2.14` low. Jack detection logic ignores physical jack pins during Bluetooth streaming to prevent false unplug events.

## Firmware Image & Naming

- **Image Container:** `ECHOV410.IMG` is a 5 MiB `RKnanoFW` container holding outer headers, row tables, code/data, header copy at `0x44C000`, and CRC-32 trailer at `0x500000`.
- **Target File Naming:** Target updater looks for `ECHOVnnn.IMG` (e.g. `ECHOV410.IMG` or FiiO's stock `ECHOV200.IMG`). Refuses `MINIVnnn.IMG` images meant for the 512 MB Echo Mini model.
- **Build Tools:** Built via `tools/make_image.py`.