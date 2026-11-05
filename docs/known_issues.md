## Audio & USB DAC Mode Issues (v0.1.0-alpha)

### 1. Audio Cracking / Popping
- **Symptom:** Audio occasionally crackles or pops, specifically noticed while operating in **USB DAC mode**. 
- **Trigger/Context:** Observed happening concurrently when receiving a Windows system notification. 
- **Status:** Unsure if isolated to USB DAC host packet handling, ring buffers, or low-level firmware buffer handling. Pending deep-dive investigation.

### 2. Swapped Channels (Left/Right Inversion)
- **Symptom:** Left and right stereo channels are inverted (swapped).
- **Verification:** Tested and confirmed using the built-in Windows sound check configuration tool.
- **Note:** Spatial sound features still function normally despite the channel swap. Requires inspection of the I2S/DMA audio pipeline or channel mapping configurations.