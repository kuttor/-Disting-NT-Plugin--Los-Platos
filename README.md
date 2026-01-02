# Los Platos v3.1.0 - in Active Development

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

**Los Platos** ("The Dishes") is a physical modeling cymbal synthesizer that goes far beyond sample playback. Using **true modal synthesis** based on spectral analysis of real cymbals, it recreates the complex physics of vibrating metal—from tight bell pings to massive washy crashes.

Four cymbal types capture the character of real metal:

- **Ride** — Edge (dark, sustained wash) and Bell (bright, focused ping)
- **Crash** — Explosive attack, bright wash, medium decay
- **Hi-Hat** — Dual-state physics with pedal control (open/closed)
- **Splash** — Quick, bright, percussive punctuation

The **two-input performance model** (Stick + Choke) gives you infinite gradients between open and choked sounds—from free-ringing cymbal swells to tight, muted stops.

---

## Features

### 🎛️ Four Cymbal Types

| Type | Character | Best For |
|------|-----------|----------|
| **Ride** | Edge: dark, sustained / Bell: bright ping | Jazz, fusion, progressive |
| **Crash** | Fast attack, bright, washy | Rock, pop, accents |
| **Hi-Hat** | Metallic, pedal-controlled open/closed | All genres, timekeeping |
| **Splash** | Quick, bright, small diameter | Fills, punctuation, accents |

### 🔔 Ride Strike Zones

The Ride cymbal features distinct **Edge** and **Bell** modes based on measured real cymbal characteristics:

| Zone | Spectral Centroid | Energy Distribution | Decay | Character |
|------|-------------------|---------------------|-------|-----------|
| **Edge** | ~800 Hz | 90% below 1kHz | 30+ seconds | Dark, washy, sustained |
| **Bell** | ~5700 Hz | 52% above 5kHz | ~1.5 seconds | Bright ping, focused, tight |

### 🎹 True Modal Synthesis Engine

Unlike sample-based or simple noise-filtered approaches, Los Platos uses **measured modal frequencies** from real cymbals:

- **16-18 resonant modes** per cymbal type
- **Inharmonic frequency ratios** (cymbals aren't harmonic!)
- **Individual decay rates** per mode (highs decay faster than lows)
- **Two-pole resonators** for efficient, accurate modal response

### 🥢 Two-Input Performance Model

**STICK** (Input 1 — Trigger + Velocity)
- Trigger: Strike the cymbal
- Velocity: How hard you hit (controls excitation energy)

**CHOKE** (Input 2 — Gate + Velocity) — *Optional*
- Gate: Held = gripping cymbal / pedal down
- Velocity: How tight your grip / how closed the hi-hat
- 0V = free/open, 5V = fully muted/closed
- Set to "None" if not using choke control

This means:
- Light choke velocity (1V) = barely touching the cymbal
- Medium choke velocity (3V) = firm grip / half-closed hi-hat
- High choke velocity (5V) = completely muted / tight closed hi-hat

### 🏛️ Venue Modeling

Optional reverb processing to place cymbals in acoustic space:

| Setting | Character |
|---------|-----------|
| **Off** | Dry, direct sound |
| **On** | Simple delay-based reverb with adjustable size |

### 🎚️ Kit Positioning

The **Pan** parameter lets you place cymbals in realistic drum kit positions:

| Cymbal | Typical Pan |
|--------|-------------|
| Hi-Hat | L40 to L60 |
| Ride | R40 to R60 |
| Crashes | L30 / R30 |
| Splash | Variable |

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
| **Strike** | Edge / Bell | Strike zone (Ride only, grayed out for others) |
| **Size** | Small / Medium / Large | Cymbal diameter (affects pitch via frequency scaling) |
| **Weight** | Thin / Heavy | Cymbal thickness (affects attack character) |
| **Tone** | 0-100% | Dark to bright (affects high frequency decay) |
| **Decay** | 0-100% | Sustain length multiplier (0.5x to 1.5x) |
| **Gain** | 0-150 | Output level (100 = unity) |

### Venue Page

| Parameter | Range | Description |
|-----------|-------|-------------|
| **Venue** | Off / On | Enable/disable reverb processing |
| **Reverb** | 0-100% | Wet/dry mix (grayed out when Venue = Off) |
| **Pan** | L50 - C - R50 | Stereo position (grayed out when Venue = Off) |

### Routing Page

| Parameter | Range | Default | Description |
|-----------|-------|---------|-------------|
| **Stick In** | 1-28 | 1 | Trigger + velocity input |
| **Choke In** | None, 1-28 | 2 | Gate + velocity for grip/pedal |
| **Stick Vel** | 0-100% | 80% | Velocity sensitivity scaling |
| **Choke Vel** | 0-100% | 50% | Choke sensitivity scaling |
| **Output L** | 1-28 | 13 | Left output bus |
| **Output R** | None, 1-28 | 14 | Right output bus (None = mono) |
| **Output Mode** | Add / Replace | Add | Sum with or replace bus content |

---

## Model Details

### Ride Bell — Focused Ping Character

Based on spectral analysis of real ride cymbal bell hits:

- **Fundamental**: 407 Hz (present but not dominant)
- **Peak "Ping"**: 4208 Hz and 5638 Hz (loudest modes)
- **Spectral Centroid**: 5700 Hz
- **Energy Distribution**: 52% above 5kHz, only 7% below 1kHz
- **Decay**: ~1.5 seconds to -40dB

The bell sound is characterized by strong high-frequency content with fast decay—producing the iconic "ping" that cuts through a mix.

### Ride Edge — Dark Sustained Wash

Based on spectral analysis of real ride cymbal edge hits:

- **Fundamental**: 102 Hz (loudest mode!)
- **Energy Distribution**: 90% below 1kHz, only 3% above 5kHz
- **Spectral Centroid**: 800 Hz
- **Decay**: 30+ seconds to -40dB

The edge activates the entire cymbal surface, producing a dark, washy sound with extremely long sustain—the opposite character of bell hits.

### Crash — Explosive Brightness

Bright, explosive attack with medium decay:

- **Frequency Range**: 280 Hz - 12.5 kHz
- **Peak Energy**: 4000-5400 Hz
- **Decay**: 2-3 seconds
- **Character**: Fast attack, bright wash, dramatic accent

### Hi-Hat — Dual-State Metal

Two distinct mode sets based on pedal position:

**Open Hi-Hat**
- Bright, metallic shimmer
- Peak energy at 7200 Hz
- ~1.4 second decay

**Closed Hi-Hat**
- Very short, tight
- Peak energy at 9000 Hz
- 25ms decay
- Automatic switching via Choke input threshold

### Splash — Quick Punctuation

Small cymbal character:

- **Frequency Range**: 600 Hz - 15 kHz
- **Peak Energy**: 6500 Hz
- **Decay**: ~0.7 seconds
- **Character**: Quick, bright, percussive

---

## Modal Synthesis Architecture

### Why Modal Synthesis?

Real cymbals produce sound through **hundreds of vibrating modes**—each with its own frequency and decay time. Unlike harmonic instruments (strings, winds), cymbal modes are **inharmonic**:

| Mode | Frequency | Ratio to Fundamental |
|------|-----------|---------------------|
| 1 | 407 Hz | 1.00x |
| 2 | 570 Hz | 1.40x (not 2x!) |
| 3 | 878 Hz | 2.16x (not 3x!) |
| 4 | 1670 Hz | 4.10x |
| 5 | 2681 Hz | 6.58x |

This inharmonicity is why cymbals sound like cymbals—and why simple synthesis approaches fail.

### Two-Pole Resonator Implementation

Each mode is implemented as a two-pole resonator:

```
y[n] = a1 * y[n-1] + a2 * y[n-2]

where:
  a1 = 2 * r * cos(ω)
  a2 = -r²
  r = decay coefficient (0.9999 for 40s decay, 0.99 for 0.1s)
  ω = 2π * frequency / sampleRate
```

This provides:
- Accurate frequency placement
- Individual decay control per mode
- Efficient CPU usage
- Zero latency

### Stick Transient

The initial "click" of stick on metal:

- **Duration**: 0.8ms (heavy stick) to 3ms (soft mallet)
- **Character**: Filtered noise burst
- **Brightness**: Varies by cymbal type (bell = bright, edge = dark)

---

## Usage Tips

### Getting Started

1. Set **Type** to Ride, **Strike** to Bell
2. Set **Size** to Medium, **Weight** to Heavy
3. Trigger with a gate signal
4. Adjust **Gain** to taste

### Ride Bell Ping

1. **Type**: Ride, **Strike**: Bell
2. **Size**: Medium or Large
3. **Tone**: 60-80% for bright ping
4. **Decay**: 40-60%
5. **Venue**: Off or minimal reverb

### Ride Edge Wash

1. **Type**: Ride, **Strike**: Edge
2. **Size**: Large for maximum sustain
3. **Tone**: 30-50% for dark wash
4. **Decay**: 70-100%
5. Let it ring—this is the sustained "ride" sound

### Jazz Ride Pattern

1. **Type**: Ride, alternate **Strike** between Edge and Bell
2. Use velocity variation for dynamics
3. **Venue**: On with 20-30% reverb
4. **Pan**: R50 for realistic kit position

### Tight Hi-Hat

1. **Type**: Hi-Hat
2. Connect **Choke In** to pedal CV
3. High choke velocity = closed, low = open
4. **Venue**: Off for tight electronic sound

### Crash Accent

1. **Type**: Crash
2. **Size**: Large for explosive wash
3. **Decay**: 50-70%
4. **Venue**: On with 30-50% reverb
5. Single hits for dramatic accents

### Building a Kit

Set up multiple Los Platos instances:
- Hi-Hat at L50, Ride at R50
- Crashes at L30 and R30
- All sharing same **Venue** settings for cohesion

---

## Technical Specifications

| Specification | Value |
|---------------|-------|
| Modal Resonators | 16-18 per cymbal type |
| Sample Rate | Follows system (48kHz typical) |
| Memory Usage | ~8KB (resonator state) |
| Latency | Zero (direct synthesis) |
| Trigger Threshold | >0.5V |
| Choke Threshold | Continuous 0-5V |

### Modal Frequency Data

**Ride Bell** (18 modes): 407 Hz - 10 kHz
**Ride Edge** (16 modes): 47 Hz - 1.5 kHz  
**Crash** (13 modes): 280 Hz - 12.5 kHz
**Hi-Hat Open** (11 modes): 440 Hz - 15 kHz
**Hi-Hat Closed** (6 modes): 1.5 kHz - 15 kHz
**Splash** (8 modes): 600 Hz - 15 kHz

### Decay Time Ranges

| Cymbal | Shortest Mode | Longest Mode |
|--------|---------------|--------------|
| Ride Bell | 0.04s | 2.0s |
| Ride Edge | 2.0s | 40.0s |
| Crash | 0.08s | 3.0s |
| Hi-Hat Open | 0.08s | 1.0s |
| Hi-Hat Closed | 0.015s | 0.05s |
| Splash | 0.07s | 0.6s |

---

## Signal Flow

```
                    ┌─────────────────────────────────────────┐
                    │              LOS PLATOS                 │
                    │                                         │
  STICK ────────────┤  ┌─────────────────────────────────────┐│
  (Trig + Vel)      │  │         STICK TRANSIENT             ││
                    │  │  ┌─────────────────────────────┐    ││
                    │  │  │ Noise Burst (0.8-3ms)       │    ││
                    │  │  │ Brightness varies by type   │    ││
                    │  │  │ Velocity-scaled amplitude   │    ││
                    │  │  └──────────────┬──────────────┘    ││
                    │  └─────────────────│───────────────────┘│
                    │                    │                    │
                    │  ┌─────────────────│───────────────────┐│
                    │  │         MODAL BANK (16-18 modes)    ││
                    │  │                 │                   ││
                    │  │    ┌────────────┴────────────┐      ││
                    │  │    │                         │      ││
                    │  │  ┌─┴─┐ ┌───┐ ┌───┐     ┌───┐│      ││
                    │  │  │ M │ │ M │ │ M │ ... │ M ││      ││
                    │  │  │ 1 │ │ 2 │ │ 3 │     │ N ││      ││
                    │  │  └─┬─┘ └─┬─┘ └─┬─┘     └─┬─┘│      ││
                    │  │    │     │     │         │  │      ││
                    │  │    │  Each mode has:     │  │      ││
                    │  │    │  • Unique frequency │  │      ││
                    │  │    │  • Own decay rate   │  │      ││
                    │  │    │  • Amplitude weight │  │      ││
                    │  │    │     │     │         │  │      ││
                    │  │    └─────┴─────┴────┬────┘  │      ││
                    │  │                     │       │      ││
                    │  │              ┌──────┴──────┐│      ││
                    │  │              │   Summer    ││      ││
                    │  │              │ (normalize) ││      ││
                    │  │              └──────┬──────┘│      ││
                    │  └─────────────────────│───────┘      │
                    │                        │              │
                    │                   ┌────┴────┐         │
                    │                   │  Mixer  │         │
                    │                   │(+click) │         │
                    │                   └────┬────┘         │
                    │                        │              │
  CHOKE ────────────┤  ┌─────────────────────┴─────────────┐│
  (Gate + Vel)      │  │           CHOKE ENGINE            ││
                    │  │  ┌─────────────────────────────┐  ││
                    │  │  │ Smoothed pressure tracking  │  ││
                    │  │  │ Continuous modal damping    │  ││
                    │  │  │ Hi-hat mode switching       │  ││
                    │  │  │ (open↔closed @ threshold)   │  ││
                    │  │  └─────────────────────────────┘  ││
                    │  └───────────────────┬───────────────┘│
                    │                      │                │
                    │                 ┌────┴────┐           │
                    │                 │Soft Clip│           │
                    │                 │ (tanh)  │           │
                    │                 └────┬────┘           │
                    │                      │                │
                    │  ┌───────────────────┴───────────────┐│
                    │  │         VENUE (Optional)          ││
                    │  │  ┌─────────────────────────────┐  ││
                    │  │  │ Simple delay reverb         │  ││
                    │  │  │ Adjustable size/mix         │  ││
                    │  │  │ Stereo pan control          │  ││
                    │  │  └─────────────────────────────┘  ││
                    │  └───────────────────┬───────────────┘│
                    │                      │                │
                    │                 ┌────┴────┐           │
                    │                 │  Pan    │           │
                    │                 │ L ◄─► R │           │
                    │                 └────┬────┘           │
                    │                      │                │
                    │                 ┌────┴────┐           │
                    │                 │ Output  │           │
                    │                 │Add/Repl │           │
                    │                 └────┬────┘           │
                    └──────────────────────┼────────────────┘
                                           │
                                     OUTPUT L/R
```

---

## Two-Pole Resonator Detail

```
                 SINGLE MODAL RESONATOR
    ┌────────────────────────────────────────────┐
    │                                            │
    │  Strike Energy                             │
    │       │                                    │
    │       ▼                                    │
    │    ┌──────┐                                │
    │    │ y[n] │◄──────────────────────────┐    │
    │    └──┬───┘                           │    │
    │       │                               │    │
    │       ▼                               │    │
    │    ┌──────┐    ┌──────┐    ┌──────┐   │    │
    │    │  +   │◄───│ a1×  │◄───│ z⁻¹ │◄──┤    │
    │    └──┬───┘    └──────┘    └──────┘   │    │
    │       │                     y[n-1]    │    │
    │       ▼                               │    │
    │    ┌──────┐    ┌──────┐    ┌──────┐   │    │
    │    │  +   │◄───│ a2×  │◄───│ z⁻¹ │◄──┘    │
    │    └──┬───┘    └──────┘    └──────┘        │
    │       │                     y[n-2]         │
    │       ▼                                    │
    │    Output                                  │
    │                                            │
    │  a1 = 2 × r × cos(2πf/sr)                  │
    │  a2 = -r²                                  │
    │  r = 0.001^(1/(decay × sr))                │
    │                                            │
    └────────────────────────────────────────────┘
```

---

## Changelog

### v3.1.0 (January 2026)
- 🔧 **Edge vs Bell Fix** — Completely reversed in v3.0, now correct:
  - Edge: dark (791 Hz centroid), 90% energy below 1kHz, 30s+ decay
  - Bell: bright (5706 Hz centroid), 52% energy above 5kHz, 1.5s decay
- ➕ **Output Mode** — Add/Replace parameter (matches other Disting NT plugins)
- 🔇 **None Options** — Choke In and Output R can now be set to None
- 🎛️ **Stick Transient** — Brightness now varies by cymbal type

### v3.0.0 (January 2026)
- 🎹 **Complete Rewrite** — True modal synthesis replacing broken resonator bank
- 📊 **Measured Mode Data** — Frequencies from spectral analysis of real cymbals
- ⏱️ **Per-Mode Decay** — Individual decay rates (highs fast, lows slow)
- 🔢 **Inharmonic Ratios** — Proper cymbal frequency relationships

### v2.x (December 2025)
- Multiple attempts to fix resonator-based synthesis
- Issues: wrong spectral centroid, decay times 4-6x too long
- All cymbal types sounded similar (fundamental architecture problem)

### v1.x (December 2025)
- Initial release with exponential resonator spreading
- Issues discovered through spectral analysis comparison with real cymbals

---

## Roadmap

Features planned for future versions:

- [ ] **China Cymbal** — Trashy, inverted edge character
- [ ] **Sizzle/Rivet Modes** — Sympathetic resonator chains
- [ ] **Zone CV Input** — CV control of strike position
- [ ] **Bow Strike Zone** — Middle position between edge and bell
- [ ] **Physical Venue Modeling** — Port FDN reverb from El Applauso
- [ ] **Custom UI** — Output meters and activity visualization
- [ ] **Grab Transient** — "Chk" sound when grabbing ringing cymbal

---

## Research Foundation

### Spectral Analysis Methodology

Mode frequencies were derived from FFT analysis of acoustic cymbal recordings:

1. **Windowed FFT** — 0.2s attack portion with Hanning window
2. **Peak Detection** — Prominence-based mode identification
3. **Decay Measurement** — Per-band envelope analysis
4. **Centroid Calculation** — Energy-weighted frequency average

### Key Findings

| Characteristic | Bell | Edge |
|----------------|------|------|
| Spectral Centroid | 5706 Hz | 791 Hz |
| Energy <1kHz | 7% | 90% |
| Energy >5kHz | 52% | 3% |
| -40dB Decay | 1.5s | 30s+ |
| Dominant Modes | 4-6 kHz | 50-500 Hz |

### Academic References

- Peterson & Rossing — Cymbal modal frequency distribution (1982)
- Fletcher & Bassett — 600+ identifiable frequencies in cymbals (1979)
- Bilbao — Nonlinear plate modeling for realistic cymbal synthesis
- Perrin et al. — Nonlinear harmonics above 20µm displacement (2008)

---

## Credits

**Los Platos** was developed for the Expert Sleepers Disting NT platform.

Physical modeling informed by:
- Spectral analysis of real cymbal recordings
- Peterson & Rossing cymbal modal analysis (1982)
- Fletcher & Bassett acoustic studies (1979)
- The satisfying "ping" of a well-struck bell 🔔

---

## License

This plugin is provided as-is for use with the Expert Sleepers Disting NT.

---

## Support

For bug reports, feature requests, or general feedback—your input shapes future versions.

---

<div align="center">

```
        🥁🥁🥁🥁🥁🥁🥁🥁
      🥁                🥁
     🥁   RIDE  CRASH   🥁
     🥁    HI-HAT       🥁
      🥁     SPLASH     🥁
        🥁🥁🥁🥁🥁🥁🥁🥁
```

**Los Platos** — *The Dishes Have Never Sounded This Good.*

</div>
