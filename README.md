# Los Platos v0.1.0-dev

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

## ⚠️ DEVELOPMENT BUILD

This is a development build (v0.1.0-dev). Features may change before release.

---

## Overview

**Los Platos** ("The Dishes") is a physical modeling cymbal synthesizer using **true modal synthesis** based on spectral analysis of real cymbals. Each cymbal type uses measured modal frequencies with individual decay rates per mode, resulting in authentic cymbal behavior.

**Currently implemented cymbal types:**

- **Ride** — Edge (dark, washy) and Bell (bright ping)
- **Crash** — Explosive attack, dense inharmonic wash
- **Hi-Hat** — Open/closed with realistic choke pedal control
- **Splash** — Bright, quick, with proper body

---

## What's New in v0.1.0-dev

### Major Fixes

| Issue | Problem | Fix |
|-------|---------|-----|
| **Hi-Hat "Open" sounded closed** | Decay 60-180ms (closed range), continuous micro-choking | Decay 400-700ms, threshold-based choke damping |
| **Splash sounded like a tick** | No modes below 5kHz, missing body | Added 1-4kHz body modes, 25 total modes |
| **Crash too pitched/metallic** | Orderly frequency progression | Inharmonic jitter, 46-mode dense distribution |
| **Reverb was primitive** | Simple delay-based fake reverb | Full FDN reverb from El Applauso |

### New FDN Reverb

Ported from El Applauso v2.3.2 - a proper feedback delay network with:

| Venue | Character | RT60 | Best For |
|-------|-----------|------|----------|
| **Studio** | Tight, controlled | 0.3s | Close-mic sound |
| **Garage** | Reflective, slappy | 1.8s | Raw punk/rock |
| **Bar** | Warm, mid-focused | 0.95s | Jazz, intimate |
| **Basement** | Dark, resonant | 2.2s | Industrial, dub |
| **Arena** | Massive, diffuse | 3.8s | Epic, cinematic |

Features: Early reflections, tank modulation, modal emphasis, air absorption modeling.

---

## Features

### 🎛️ Four Cymbal Types

| Type | Modes | Character | Best For |
|------|-------|-----------|----------|
| **Ride** | 17-19 | Edge: dark wash / Bell: bright ping | Jazz, fusion |
| **Crash** | 46 | Dense inharmonic fog, explosive | Rock, pop, accents |
| **Hi-Hat** | 44/16 | Open: long shimmer / Closed: tight tick | All genres |
| **Splash** | 25 | Bright with body, quick decay | Fills, punctuation |

### 🔔 Ride Strike Zones

| Zone | Centroid | Decay | Character |
|------|----------|-------|-----------|
| **Edge** | ~2500 Hz | 1-3s | Dark, washy |
| **Bell** | ~5300 Hz | ~1.5s | Bright ping |

### 🥢 Two-Input Performance Model

**STICK** (Input 1 — Trigger + Velocity)
- Trigger: Strike the cymbal
- Velocity: How hard you hit

**CHOKE** (Input 2 — Gate + Velocity) — *Optional*
- Gate: Held = gripping cymbal / pedal down
- Threshold-based damping (no micro-choke when open)
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
| **Size** | Small / Medium / Large | Cymbal diameter |
| **Style** | Normal / Heavy / Sizzle | Weight/character (Ride only) |
| **Tone** | 0-100% | Dark to bright |
| **Decay** | 0-100% | Sustain length |
| **Gain** | 0-150 | Output level |

### Venue Page

| Parameter | Range | Description |
|-----------|-------|-------------|
| **Venue** | Off / On | Enable reverb |
| **Type** | Studio / Garage / Bar / Basement / Arena | Room character |
| **Amount** | 0-100% | Wet/dry mix |
| **Pan** | L50 - C - R50 | Stereo position |

### Routing Page

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| **Stick In** | 1-28 | 1 | Trigger + velocity input |
| **Choke In** | None, 1-28 | None | Gate for grip/pedal |
| **Stick Vel** | 0-100% | 80% | Velocity sensitivity |
| **Choke Vel** | 0-100% | 50% | Choke sensitivity |
| **Output L** | 1-28 | 13 | Left output |
| **Output R** | None, 1-28 | 14 | Right output |
| **Output Mode** | Add / Replace | Add | Sum or replace bus |

---

## Technical Details

### Modal Synthesis Engine

- Up to 50 modes per cymbal type
- Individual frequency, amplitude, and decay per mode
- Inharmonic ratios for realistic cymbal "fog"
- Frequency-dependent decay (highs fade faster)

### Choke Behavior (v0.1.0 Fix)

```
Pressure < 0.3:  No damping (let it ring!)
Pressure 0.3-0.7: Gentle quadratic damping
Pressure > 0.7:  Strong damping (closing)
```

This prevents the "micro-choke" issue where open cymbals were constantly damped.

### Mode Counts by Type

| Type | Open | Closed | Notes |
|------|------|--------|-------|
| Ride Bell | 19 | - | Sparse, pingy |
| Ride Edge | 17 | - | Dark wash |
| Crash | 46 | - | Dense inharmonic |
| Hi-Hat | 44 | 16 | Long/short decay |
| Splash | 25 | - | Body + sparkle |

---

## Changelog

### v0.1.0-dev (January 2026)
- **FIXED**: Hi-Hat open decay too short (60-180ms → 400-700ms)
- **FIXED**: Choke damping was continuous, now threshold-based
- **FIXED**: Splash missing body modes (added 1-4kHz)
- **FIXED**: Crash too pitched (added inharmonic jitter)
- **NEW**: FDN Reverb from El Applauso v2.3.2
- **NEW**: 5 venue types with distinct characters
- Increased wash sustain for open hi-hat (0.25s → 0.9s)

### v3.7.0 (January 2026)
- Fixed modal normalization (was dividing by sqrt(numModes))
- Reduced wash noise levels
- Raised Ride Edge frequencies
- Denser Crash mode distribution

### v3.6.x (January 2026)
- Hi-Hat rewritten from spectral analysis
- WashNoise filter fixed
- Amplitude distribution corrected

### v3.0.0 (January 2026)
- Complete rewrite using true modal synthesis
- Measured modal frequencies from real cymbals

---

## Known Issues

- [ ] High CPU with 46-mode Crash on some patches
- [ ] Ride Sizzle style needs tuning
- [ ] No CV input for strike zone selection yet

---

## Roadmap

- [ ] China cymbal type
- [ ] Per-hit frequency randomization
- [ ] Custom UI with spectrum display
- [ ] CV-controlled strike zone

---

## Credits

**Los Platos** was developed for the Expert Sleepers Disting NT platform.

Physical modeling informed by:
- Spectral analysis of real cymbal recordings
- Peterson & Rossing cymbal modal analysis (1982)
- Fletcher & Bassett acoustic studies (1979)

FDN Reverb ported from El Applauso v2.3.2.

---

## License

This plugin is provided as-is for use with the Expert Sleepers Disting NT.

---

<div align="center">

**Los Platos** — *The Dishes Have Never Sounded This Good.*

</div>
