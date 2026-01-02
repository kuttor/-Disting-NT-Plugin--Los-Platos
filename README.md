# Los Platos v3.1.0

### A Physical Modeling Cymbal Synthesizer for Expert Sleepers Disting NT

---

```
    ██╗      ██████╗ ███████╗                                                
    ██║     ██╔═══██╗██╔════╝                                                
    ██║     ██║   ██║███████╗                                                
    ██║     ██║   ██║╚════██║                                                
    ███████╗╚██████╔╝███████║                                                
    ╚══════╝ ╚═════╝ ╚══════╝                                                
                                                                             
    ██████╗ ██╗      █████╗ ████████╗ ██████╗ ███████╗                       
    ██╔══██╗██║     ██╔══██╗╚══██╔══╝██╔═══██╗██╔════╝                       
    ██████╔╝██║     ███████║   ██║   ██║   ██║███████╗                       
    ██╔═══╝ ██║     ██╔══██║   ██║   ██║   ██║╚════██║                       
    ██║     ███████╗██║  ██║   ██║   ╚██████╔╝███████║                       
    ╚═╝     ╚══════╝╚═╝  ╚═╝   ╚═╝    ╚═════╝ ╚══════╝                       
                                        
                        🥁 The Dishes 🥁
```

---

## Overview

**Los Platos** ("The Dishes") is a physical modeling cymbal synthesizer using **true modal synthesis** based on spectral analysis of real cymbals. Each cymbal type uses measured modal frequencies with individual decay rates per mode, resulting in authentic cymbal behavior.

**Currently implemented cymbal types:**

- **Ride** — Edge (dark, long sustain) and Bell (bright ping, short decay)
- **Crash** — Explosive attack, bright wash
- **Hi-Hat** — Open/closed with choke pedal control
- **Splash** — Quick, bright, percussive

---

## Features

### 🎛️ Four Cymbal Types

| Type | Character | Best For |
|------|-----------|----------|
| **Ride** | Edge: dark, sustained / Bell: bright ping | Jazz, fusion, progressive |
| **Crash** | Fast attack, bright, medium decay | Rock, pop, accents |
| **Hi-Hat** | Bright, metallic, pedal-controlled | All genres, timekeeping |
| **Splash** | Quick, bright, small diameter | Fills, punctuation |

### 🔔 Ride Strike Zones

The Ride cymbal features distinct **Edge** and **Bell** modes based on measured real cymbal characteristics:

| Zone | Spectral Centroid | Decay | Character |
|------|-------------------|-------|-----------|
| **Edge** | ~1500 Hz | Very long (30s+) | Dark, washy, sustained |
| **Bell** | ~5300 Hz | Short (~1.5s) | Bright ping, focused |

### 🥢 Two-Input Performance Model

**STICK** (Input 1 — Trigger + Velocity)
- Trigger: Strike the cymbal
- Velocity: How hard you hit

**CHOKE** (Input 2 — Gate + Velocity) — *Optional*
- Gate: Held = gripping cymbal / pedal down
- Velocity: How tight your grip / how closed the hi-hat
- 0V = free/open, 5V = fully muted/closed
- Set to "None" if not using choke control

---

## Installation

1. Copy `losPlatos.instruments` to your Disting NT SD card:
   ```
   /plugins/losPlatos.instruments
   ```

2. Power cycle your Disting NT

3. Navigate to **Instruments → Los Platos**

---

## Parameters

### Sound Page

| Parameter | Range | Description |
|-----------|-------|-------------|
| **Type** | Ride / Crash / Hi-Hat / Splash | Cymbal type selection |
| **Strike** | Edge / Bell | Strike zone (Ride only) |
| **Size** | Small / Medium / Large | Cymbal diameter (affects pitch) |
| **Weight** | Thin / Heavy | Cymbal thickness (affects attack) |
| **Tone** | 0-100% | Dark to bright character |
| **Decay** | 0-100% | Sustain length multiplier |
| **Gain** | 0-150 | Output level |

### Venue Page

| Parameter | Range | Description |
|-----------|-------|-------------|
| **Venue** | Off / On | Enable reverb processing |
| **Reverb** | 0-100% | Wet/dry mix |
| **Pan** | L50 - C - R50 | Stereo position |

### Routing Page

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| **Stick In** | 1-28 | 1 | Trigger + velocity input |
| **Choke In** | None, 1-28 | 2 | Gate + velocity for grip/pedal |
| **Stick Vel** | 0-100% | 80% | Velocity sensitivity |
| **Choke Vel** | 0-100% | 50% | Choke sensitivity |
| **Output L** | 1-28 | 13 | Left output |
| **Output R** | None, 1-28 | 14 | Right output (None = mono) |
| **Output Mode** | Add / Replace | Add | Sum with or replace bus content |

---

## Technical Details

### Modal Synthesis Engine

Los Platos uses true modal synthesis with measured frequencies from real cymbals:

- **18 modes** for Ride Bell (407 Hz - 10 kHz)
- **16 modes** for Ride Edge (47 Hz - 1.5 kHz)  
- Individual decay times per mode (highs decay faster)
- Proper inharmonic frequency ratios

### Real Cymbal Analysis

Mode frequencies were derived from spectral analysis of acoustic cymbal recordings:

**Ride Bell** (measured):
- Fundamental: 407 Hz
- Peak "ping" frequencies: 4208 Hz, 5638 Hz
- Spectral centroid: 5287 Hz
- Decay: ~1.5 seconds

**Ride Edge** (measured):
- Fundamental: 102 Hz (much lower!)
- Most energy below 1 kHz
- Spectral centroid: 1524 Hz
- Decay: 30+ seconds

---

## Changelog

### v3.1.0 (January 2026)
- **FIXED**: Edge vs Bell modes completely reversed in v3.0
  - Edge now correctly dark (1524 Hz centroid) with very long decay
  - Bell now correctly bright (5287 Hz centroid) with short decay
- Added **Output Mode** parameter (Add/Replace)
- Added **None** option for Choke input
- Added **None** option for Output R (mono mode)
- Stick transient brightness now matches cymbal type

### v3.0.0 (January 2026)
- Complete rewrite using true modal synthesis
- Measured modal frequencies from real cymbals
- Individual decay rates per mode
- Replaced broken resonator bank approach

### v2.x (December 2025)
- Multiple attempts to fix resonator-based synthesis
- Issues: wrong spectral centroid, decay times 4-6x too long
- All cymbal types sounded similar

### v1.x (December 2025)
- Initial release with resonator bank synthesis
- Issues discovered through spectral analysis comparison

---

## Roadmap

Features planned for future versions:

- [ ] China cymbal type
- [ ] Sizzle/rivet modes
- [ ] Zone CV input
- [ ] Additional venue reverb types
- [ ] Custom UI with output meters
- [ ] Grab transient synthesis

---

## Credits

**Los Platos** was developed for the Expert Sleepers Disting NT platform.

Physical modeling informed by:
- Spectral analysis of real cymbal recordings
- Peterson & Rossing cymbal modal analysis (1982)
- Fletcher & Bassett acoustic studies (1979)

---

## License

This plugin is provided as-is for use with the Expert Sleepers Disting NT.

---

<div align="center">

**Los Platos** — *The Dishes Have Never Sounded This Good.*

</div>
