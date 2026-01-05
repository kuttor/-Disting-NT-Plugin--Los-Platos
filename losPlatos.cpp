/*
 * Los Platos v0.1.1-dev - Physical Modeling Cymbal Synthesizer
 * Expert Sleepers Disting NT
 *
 * DEVELOPMENT BUILD - NOT FOR RELEASE
 *
 * v0.1.1-dev Changes (CRITICAL BUGFIX):
 *   - FIXED: Cymbal type was stuck on Ride Edge regardless of selection!
 *     - construct() was hardcoding Ride Edge config
 *     - parameterChanged() might not be called properly in all cases
 *     - Now step() detects type changes and reconfigures automatically
 *   - This should fix hi-hat, crash, splash all sounding wrong
 *
 * v0.1.0-dev Changes (from v3.7.0):
 *   - FIX: Hi-Hat "open" was reading as closed
 *     - Decay times increased from 0.12-0.18s to 0.4-0.7s
 *     - Choke damping now threshold-based, not continuous micro-choking
 *     - Wash sustain for open hi-hat increased from 0.25s to 0.9s
 *   - FIX: Splash was "tick" not cymbal
 *     - Added 1-3kHz body/clang modes (was starting at 5kHz!)
 *   - FIX: Crash too "metallic/pitched"
 *     - Added inharmonic frequency jitter for "crash fog"
 *   - NEW: FDN Reverb from El Applauso v2.3.2
 */

#include <math.h>
#include <string.h>
#include <new>
#include <distingnt/api.h>

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))
#endif

static constexpr float PI = 3.14159265358979323846f;
static constexpr float TWO_PI = 6.28318530717958647692f;

static inline float clampf(float x, float lo, float hi) {
    return x < lo ? lo : (x > hi ? hi : x);
}

// ============================================================================
// FAST RANDOM
// ============================================================================

class FastRandom {
public:
    FastRandom(uint32_t seed = 12345) : state(seed) {}
    float nextFloat() {
        state = state * 1664525 + 1013904223;
        return (float)(state >> 8) / 16777216.0f;
    }
    float nextBipolar() { return nextFloat() * 2.0f - 1.0f; }
    void seed(uint32_t s) { state = s; }
private:
    uint32_t state;
};

// ============================================================================
// MODAL RESONATOR
// ============================================================================

class ModalResonator {
public:
    void setSampleRate(float sr) { 
        sampleRate = sr; 
        computeCoefficients();
    }
    
    void setMode(float freq, float decayTime, float amp) {
        frequency = clampf(freq, 20.0f, sampleRate * 0.45f);
        decay = clampf(decayTime, 0.01f, 60.0f);
        amplitude = amp;
        computeCoefficients();
    }
    
    void strike(float velocity, float excitation) {
        float energy = velocity * excitation * amplitude;
        y1 += energy * 0.5f;
    }
    
    float process() {
        float y = a1 * y1 + a2 * y2;
        y2 = y1;
        y1 = y;
        return y;
    }
    
    void damp(float amount) {
        y1 *= (1.0f - amount);
        y2 *= (1.0f - amount);
    }
    
    void reset() { y1 = y2 = 0.0f; }
    
private:
    void computeCoefficients() {
        float w = TWO_PI * frequency / sampleRate;
        float r = powf(0.001f, 1.0f / (decay * sampleRate));
        r = clampf(r, 0.0f, 0.99999f);
        a1 = 2.0f * r * cosf(w);
        a2 = -r * r;
    }
    
    float sampleRate = 48000.0f;
    float frequency = 1000.0f;
    float decay = 1.0f;
    float amplitude = 1.0f;
    float a1 = 0.0f, a2 = 0.0f;
    float y1 = 0.0f, y2 = 0.0f;
};

// ============================================================================
// MODE DATA - Measured from real cymbals
// ============================================================================

struct ModeData {
    float freq;
    float amp;
    float decay;
};

// RIDE BELL - BRIGHT (5287 Hz centroid), SHORT decay (~1.5s)
// RIDE BELL - BRIGHT (4800 Hz centroid), focused ping character
// Boosted high freq amplitudes and extended decay times
static const ModeData RIDE_BELL_MODES[] = {
    { 407.5f,  0.15f, 2.0f },    // Reduced low mode
    { 570.0f,  0.10f, 1.8f },    
    { 878.0f,  0.12f, 1.5f },    
    { 1670.0f, 0.18f, 1.2f },
    { 2681.0f, 0.30f, 0.9f },
    { 3264.0f, 0.45f, 0.7f },
    { 3518.0f, 0.40f, 0.6f },
    { 3604.0f, 0.65f, 0.55f },
    { 4208.0f, 1.00f, 0.50f },   // LOUDEST - the ping!
    { 4389.0f, 0.55f, 0.45f },
    { 4991.0f, 0.65f, 0.40f },
    { 5578.0f, 0.60f, 0.35f },
    { 5638.0f, 0.95f, 0.30f },   // Secondary ping
    { 6043.0f, 0.75f, 0.25f },
    { 6641.0f, 0.60f, 0.20f },
    { 7500.0f, 0.45f, 0.18f },
    { 8500.0f, 0.35f, 0.15f },
    { 10000.0f, 0.25f, 0.12f },
};
static constexpr int NUM_RIDE_BELL_MODES = sizeof(RIDE_BELL_MODES) / sizeof(ModeData);

// RIDE EDGE - DARK (791 Hz centroid), VERY LONG decay (36s!)
// RIDE EDGE - Dark wash but still needs shimmer
// Real ride edge: darker than bell but still has high-frequency content
// Centroid should be around 2000-3000 Hz, not 300 Hz!
static const ModeData RIDE_EDGE_MODES[] = {
    // Low wash body
    {   280.0f, 0.25f, 3.0f },
    {   420.0f, 0.30f, 2.8f },
    {   580.0f, 0.35f, 2.5f },
    {   750.0f, 0.40f, 2.2f },
    
    // Mid body
    {  1000.0f, 0.50f, 2.0f },
    {  1350.0f, 0.55f, 1.8f },
    {  1700.0f, 0.60f, 1.6f },
    {  2100.0f, 0.70f, 1.5f },
    
    // Upper character - ride wash
    {  2600.0f, 0.75f, 1.3f },
    {  3200.0f, 0.80f, 1.2f },
    {  3800.0f, 0.85f, 1.0f },
    {  4500.0f, 0.80f, 0.9f },
    
    // Shimmer (ride edge still has this, just less than bell)
    {  5500.0f, 0.55f, 0.7f },
    {  6500.0f, 0.45f, 0.6f },
    {  7500.0f, 0.35f, 0.5f },
    {  9000.0f, 0.25f, 0.4f },
};
static constexpr int NUM_RIDE_EDGE_MODES = sizeof(RIDE_EDGE_MODES) / sizeof(ModeData);

// CRASH - Measured from real crash cymbal  
// Centroid: 2845 Hz, Decay: 8.30s
// Energy: <1kHz 67% | 1-5kHz 15% | >5kHz 18%
// Needs both low body AND high shimmer
// CRASH - v0.1.0 with INHARMONIC mode distribution
// PM Analysis: modes too orderly/linear = pitched metallic tones instead of crash fog
// Real cymbals have dense inharmonic partial clouds, not discrete tones
// Using slightly randomized/jittered frequencies based on cymbal physics
static const ModeData CRASH_MODES[] = {
    // Low wash - jittered for inharmonicity
    {   347.0f, 0.12f, 1.6f },
    {   489.0f, 0.14f, 1.5f },  // Not 480, slightly off
    {   631.0f, 0.16f, 1.4f },  // Not 630
    {   798.0f, 0.18f, 1.2f },  // Not 800
    {   967.0f, 0.22f, 1.0f },  // Not 950
    
    // Mid body - dense cluster with jitter
    {  1138.0f, 0.28f, 0.9f },
    {  1287.0f, 0.30f, 0.88f },  // Closer spacing
    {  1423.0f, 0.32f, 0.85f },
    {  1589.0f, 0.36f, 0.82f },
    {  1734.0f, 0.40f, 0.80f },
    {  1891.0f, 0.44f, 0.78f },
    {  2067.0f, 0.48f, 0.75f },
    {  2234.0f, 0.52f, 0.72f },
    {  2418.0f, 0.56f, 0.70f },
    {  2589.0f, 0.58f, 0.68f },
    {  2778.0f, 0.62f, 0.65f },
    {  2961.0f, 0.64f, 0.62f },
    
    // Upper-mid - VERY dense for "crash fog"
    {  3156.0f, 0.68f, 0.60f },
    {  3298.0f, 0.70f, 0.58f },  // Only 142 Hz apart
    {  3467.0f, 0.74f, 0.56f },
    {  3612.0f, 0.76f, 0.54f },
    {  3789.0f, 0.80f, 0.52f },
    {  3934.0f, 0.82f, 0.50f },
    {  4123.0f, 0.86f, 0.48f },
    {  4298.0f, 0.88f, 0.46f },
    {  4489.0f, 0.92f, 0.44f },
    {  4667.0f, 0.94f, 0.42f },
    {  4878.0f, 0.96f, 0.40f },
    {  5067.0f, 0.98f, 0.38f },
    {  5289.0f, 1.00f, 0.38f },  // LOUDEST
    {  5478.0f, 0.96f, 0.36f },
    {  5712.0f, 0.92f, 0.34f },
    
    // Presence/shimmer - still dense
    {  5934.0f, 0.82f, 0.32f },
    {  6189.0f, 0.75f, 0.30f },
    {  6423.0f, 0.70f, 0.28f },
    {  6712.0f, 0.65f, 0.26f },
    {  6978.0f, 0.60f, 0.24f },
    {  7289.0f, 0.55f, 0.22f },
    {  7623.0f, 0.50f, 0.20f },
    {  8012.0f, 0.45f, 0.18f },
    {  8456.0f, 0.40f, 0.16f },
    {  8934.0f, 0.35f, 0.14f },
    {  9489.0f, 0.30f, 0.12f },
    { 10123.0f, 0.25f, 0.10f },
    { 11234.0f, 0.20f, 0.08f },
};
static constexpr int NUM_CRASH_MODES = sizeof(CRASH_MODES) / sizeof(ModeData);

// HI-HAT OPEN - v0.1.0 with REALISTIC decay times
// PM Analysis: "Open" was reading as closed because decay 0.06-0.18s is closed-hat range
// Real open hi-hat: 400ms-1s+ decay, not 60-180ms!
// Also: body modes need to be present but quiet - they give "clang" character
static const ModeData HIHAT_OPEN_MODES[] = {
    // BODY (300-1000 Hz) - quiet but present for "clang"
    {   438.0f, 0.10f, 0.70f },  // Body clang - longer decay now!
    {   567.0f, 0.08f, 0.65f },
    {   723.0f, 0.06f, 0.60f },
    {   891.0f, 0.05f, 0.55f },
    
    // BITE (1000-3000 Hz) - gives attack definition
    {  1245.0f, 0.10f, 0.50f },
    {  1567.0f, 0.12f, 0.48f },
    {  2102.0f, 0.15f, 0.45f },
    {  2456.0f, 0.12f, 0.42f },
    {  2834.0f, 0.10f, 0.40f },
    
    // UPPER-MID (3000-6000 Hz) - main sizzle
    {  3127.0f, 0.25f, 0.38f },
    {  3456.0f, 0.30f, 0.36f },
    {  3789.0f, 0.35f, 0.34f },
    {  4120.0f, 0.40f, 0.32f },
    {  4212.0f, 0.38f, 0.32f },
    {  4567.0f, 0.42f, 0.30f },
    {  4982.0f, 0.45f, 0.28f },
    {  5365.0f, 0.50f, 0.26f },
    {  5435.0f, 0.55f, 0.26f },
    {  5548.0f, 0.55f, 0.25f },
    {  5728.0f, 0.45f, 0.25f },
    
    // PRESENCE (6000-10000 Hz) - THE CHARACTER - needs long decay!
    {  6018.0f, 0.70f, 0.50f },
    {  6158.0f, 0.60f, 0.48f },
    {  6345.0f, 0.65f, 0.46f },
    {  6568.0f, 0.80f, 0.44f },
    {  6648.0f, 0.75f, 0.42f },
    {  6708.0f, 1.00f, 0.55f },  // LOUDEST - primary shimmer, LONG decay
    {  6775.0f, 0.70f, 0.40f },
    {  7234.0f, 0.60f, 0.38f },
    {  7567.0f, 0.55f, 0.36f },
    {  7995.0f, 0.65f, 0.34f },
    {  8380.0f, 0.70f, 0.32f },
    {  8756.0f, 0.55f, 0.30f },
    {  9234.0f, 0.50f, 0.28f },
    {  9665.0f, 0.55f, 0.26f },
    
    // AIR (10000-18000 Hz) - sparkle
    { 10234.0f, 0.50f, 0.24f },
    { 10890.0f, 0.55f, 0.22f },
    { 11770.0f, 0.55f, 0.20f },
    { 12235.0f, 0.50f, 0.18f },
    { 12890.0f, 0.45f, 0.16f },
    { 13567.0f, 0.40f, 0.14f },
    { 14234.0f, 0.35f, 0.12f },
    { 15000.0f, 0.30f, 0.10f },
    { 16000.0f, 0.25f, 0.08f },
};
static constexpr int NUM_HIHAT_OPEN_MODES = sizeof(HIHAT_OPEN_MODES) / sizeof(ModeData);

// HI-HAT CLOSED - Same frequency profile as open but MUCH shorter decay (30-50ms)
// Closed hat is the quick "tick" - still needs HF dominance
static const ModeData HIHAT_CLOSED_MODES[] = {
    // Body - very quiet and short
    {   438.0f, 0.05f, 0.025f },
    {   723.0f, 0.04f, 0.025f },
    
    // Bite - quiet
    {  1567.0f, 0.08f, 0.022f },
    {  2102.0f, 0.10f, 0.022f },
    
    // Upper-mid
    {  4120.0f, 0.35f, 0.020f },
    {  4982.0f, 0.40f, 0.020f },
    {  5435.0f, 0.50f, 0.018f },
    {  5548.0f, 0.50f, 0.018f },
    
    // Presence - this is the "tick"
    {  6018.0f, 0.65f, 0.016f },
    {  6568.0f, 0.70f, 0.015f },
    {  6708.0f, 0.85f, 0.014f },  // Main tick
    {  7995.0f, 0.55f, 0.012f },
    {  8380.0f, 0.60f, 0.012f },
    
    // Air
    { 10234.0f, 0.45f, 0.010f },
    { 12235.0f, 0.40f, 0.008f },
};
static constexpr int NUM_HIHAT_CLOSED_MODES = sizeof(HIHAT_CLOSED_MODES) / sizeof(ModeData);

// SPLASH - Measured from real splash cymbal
// Centroid: 10407 Hz, Decay: 0.63s
// EXTREMELY BRIGHT - 88% energy above 5kHz! The brightest cymbal.
// Peaks from real: 8528, 6396, 5806, 6196, 7230
// SPLASH - v0.1.0 with proper body/clang modes
// PM Analysis: "tick" because no modes below 5kHz - missing cymbal identity band
// Real splash is bright but needs 1-4kHz body to not sound like a click
// Also needs longer decay to "splash" not "tick"
static const ModeData SPLASH_MODES[] = {
    // BODY/CLANG (1-3 kHz) - gives cymbal identity, low amplitude but meaningful decay
    {  1200.0f, 0.15f, 0.60f },
    {  1580.0f, 0.18f, 0.55f },
    {  1950.0f, 0.22f, 0.52f },
    {  2380.0f, 0.25f, 0.48f },
    {  2850.0f, 0.30f, 0.45f },
    
    // UPPER-MID (3-5 kHz) - transition zone
    {  3350.0f, 0.40f, 0.50f },
    {  3900.0f, 0.50f, 0.48f },
    {  4500.0f, 0.55f, 0.45f },
    {  5075.0f, 0.60f, 0.55f },
    {  5828.0f, 0.75f, 0.55f },
    
    // PRESENCE (6-10 kHz) - main brightness
    {  6196.0f, 0.70f, 0.52f },
    {  6342.0f, 0.75f, 0.50f },
    {  6592.0f, 0.70f, 0.48f },
    {  7230.0f, 0.65f, 0.45f },
    {  7850.0f, 0.70f, 0.42f },
    {  8528.0f, 0.85f, 0.40f },
    {  9285.0f, 0.75f, 0.38f },
    {  9817.0f, 0.80f, 0.36f },
    
    // AIR (10+ kHz) - sparkle
    { 10270.0f, 0.85f, 0.34f },
    { 11500.0f, 0.90f, 0.32f },
    { 12848.0f, 1.00f, 0.35f },  // LOUDEST
    { 13502.0f, 0.75f, 0.30f },
    { 14500.0f, 0.60f, 0.28f },
    { 15500.0f, 0.50f, 0.25f },
};
static constexpr int NUM_SPLASH_MODES = sizeof(SPLASH_MODES) / sizeof(ModeData);

// ============================================================================
// MODAL BANK
// ============================================================================

static constexpr int MAX_MODES = 50;  // Increased for hi-hat (44 modes)

class ModalBank {
public:
    void setSampleRate(float sr) {
        sampleRate = sr;
        for (int i = 0; i < MAX_MODES; i++) {
            resonators[i].setSampleRate(sr);
        }
    }
    
    void configure(const ModeData* modes, int count, float sizeMod, float toneMod, float decayMod) {
        numModes = (count > MAX_MODES) ? MAX_MODES : count;
        // FIXED: Larger cymbal = lower frequencies (inverted)
        float freqMult = 1.2f - sizeMod * 0.4f;  // Small=1.2x, Large=0.8x
        float decayScale = 0.5f + decayMod * 1.0f;
        
        for (int i = 0; i < numModes; i++) {
            float freq = modes[i].freq * freqMult;
            float decay = modes[i].decay * decayScale;
            // Tone: 0% = dark (high freq decay faster), 100% = bright (high freq sustain)
            // At 50% tone, no modification
            if (freq > 3000.0f) {
                decay *= (0.7f + toneMod * 0.6f);  // 0.7x to 1.3x based on tone
            }
            resonators[i].setMode(freq, decay, modes[i].amp);
        }
    }
    
    void strike(float velocity) {
        for (int i = 0; i < numModes; i++) {
            float excitation = 0.8f + 0.4f * ((float)((i * 7 + 13) % 17) / 17.0f);
            resonators[i].strike(velocity * 1.5f, excitation);  // Moderate boost
        }
    }
    
    void damp(float amount) {
        for (int i = 0; i < numModes; i++) {
            resonators[i].damp(amount);
        }
    }
    
    float process() {
        float sum = 0.0f;
        for (int i = 0; i < numModes; i++) {
            sum += resonators[i].process();
        }
        // Gentler normalization - don't divide by sqrt(numModes)!
        // Just a fixed scaling to prevent clipping
        return sum * 0.15f;
    }
    
    void reset() {
        for (int i = 0; i < MAX_MODES; i++) {
            resonators[i].reset();
        }
    }
    
private:
    float sampleRate = 48000.0f;
    ModalResonator resonators[MAX_MODES];
    int numModes = 0;
};

// ============================================================================
// WASH NOISE - The "shhh" character of cymbals
// ============================================================================

class WashNoise {
public:
    void setSampleRate(float sr) { 
        sampleRate = sr;
        // DC-blocking high-pass coefficient
        hpCoeff = 0.995f;  // ~38 Hz cutoff - blocks DC but passes audio
    }
    
    void trigger(float velocity, float brightness, float sustain) {
        level = velocity;
        this->brightness = clampf(brightness, 0.1f, 1.0f);
        decayRate = 1.0f - (1.0f / (clampf(sustain, 0.01f, 10.0f) * sampleRate));
        decayRate = clampf(decayRate, 0.9f, 0.99999f);
    }
    
    float process(FastRandom& rng) {
        if (level < 0.0001f) return 0.0f;
        
        // Generate noise
        float noise = rng.nextBipolar();
        
        // DC-blocking high-pass: y = x - x_prev + R * y_prev
        float hp = noise - lastNoise + hpCoeff * hpState;
        hpState = hp;
        lastNoise = noise;
        
        // Brightness lowpass
        lpState += (hp - lpState) * brightness;
        
        // Apply envelope
        float output = lpState * level;
        level *= decayRate;
        
        return output;
    }
    
    void damp(float amount) {
        level *= (1.0f - amount);
    }
    
    void reset() { level = 0.0f; lpState = 0.0f; lastNoise = 0.0f; hpState = 0.0f; }
    
private:
    float sampleRate = 48000.0f;
    float level = 0.0f;
    float brightness = 0.8f;
    float decayRate = 0.999f;
    float hpCoeff = 0.995f;
    float hpState = 0.0f;
    float lpState = 0.0f;
    float lastNoise = 0.0f;
};

// ============================================================================
// STICK TRANSIENT
// ============================================================================

class StickTransient {
public:
    void setSampleRate(float sr) { sampleRate = sr; }
    
    void trigger(float velocity, float brightness) {
        phase = 0;
        this->velocity = velocity;
        duration = (int)((0.001f + (1.0f - brightness) * 0.004f) * sampleRate);
        this->brightness = brightness;
    }
    
    float process(FastRandom& rng) {
        if (phase >= duration) return 0.0f;
        float env = 1.0f - (float)phase / (float)duration;
        env = env * env;
        float noise = rng.nextBipolar();
        float filtered = noise - lastNoise * (1.0f - brightness);
        lastNoise = noise;
        phase++;
        return filtered * env * velocity * 0.25f;  // Balanced level
    }
    
    void reset() { phase = duration; }
    
private:
    float sampleRate = 48000.0f;
    int phase = 0, duration = 100;
    float velocity = 1.0f, brightness = 0.7f;
    float lastNoise = 0.0f;
};

// ============================================================================
// CYMBAL ENGINE
// ============================================================================

class CymbalEngine {
public:
    void setSampleRate(float sr) {
        sampleRate = sr;
        modalBank.setSampleRate(sr);
        stick.setSampleRate(sr);
        wash.setSampleRate(sr);
        rng.seed(42);
    }
    
    void configure(int type, int strikeType, int size, int style, float tone, float decay) {
        this->type = type;
        this->strikeType = strikeType;
        this->style = style;
        
        float sizeMod = (float)size / 2.0f;
        float toneMod = tone / 100.0f;
        float decayMod = decay / 100.0f;
        
        // Style modifiers (only applied to Ride)
        float lowBoost = 1.0f;
        float decayBoost = 1.0f;
        if (type == 0 && style == 1) {  // Heavy
            lowBoost = 1.5f;
            decayBoost = 1.3f;
        }
        sizzleEnabled = (type == 0 && style == 2);
        
        switch (type) {
            case 0:  // Ride
                if (strikeType == 1) {  // Bell
                    modalBank.configure(RIDE_BELL_MODES, NUM_RIDE_BELL_MODES, sizeMod, toneMod, decayMod * decayBoost);
                    stickBrightness = 0.8f;
                } else {  // Edge
                    modalBank.configure(RIDE_EDGE_MODES, NUM_RIDE_EDGE_MODES, sizeMod, toneMod, decayMod * decayBoost);
                    stickBrightness = 0.4f;
                }
                break;
            case 1:  // Crash
                modalBank.configure(CRASH_MODES, NUM_CRASH_MODES, sizeMod, toneMod, decayMod);
                stickBrightness = 0.7f;
                break;
            case 2:  // Hi-Hat
                currentHiHatOpen = true;
                modalBank.configure(HIHAT_OPEN_MODES, NUM_HIHAT_OPEN_MODES, sizeMod, toneMod, decayMod);
                stickBrightness = 0.95f;
                break;
            case 3:  // Splash
                modalBank.configure(SPLASH_MODES, NUM_SPLASH_MODES, sizeMod, toneMod, decayMod);
                stickBrightness = 0.98f;
                break;
        }
        
        this->sizeMod = sizeMod;
        this->toneMod = toneMod;
        this->decayMod = decayMod;
    }
    
    void trigger(float velocity) {
        modalBank.strike(velocity);
        stick.trigger(velocity, stickBrightness);
        
        // Trigger wash noise - INCREASED SUSTAIN for open sounds
        // PM Analysis: open hi-hat wash was only 0.25s, needs 0.8-1.2s
        switch (type) {
            case 1:  // Crash - full sustain
                wash.trigger(velocity * 0.18f, 0.7f, 1.8f);
                break;
            case 2:  // Hi-Hat - MUCH longer sustain for open
                wash.trigger(velocity * 0.15f, 0.85f, currentHiHatOpen ? 0.9f : 0.03f);
                break;
            case 3:  // Splash - longer sustain to ring
                wash.trigger(velocity * 0.12f, 0.90f, 0.5f);
                break;
        }
        
        if (sizzleEnabled) {
            sizzleLevel = velocity;  // Start sizzle at strike velocity
        }
    }
    
    void updateChoke(float pressure) {
        // PM Analysis: continuous damping at pressure > 0.05 was "micro-choking" open hats
        // Fix: Only apply damping when actually closing, not just slightly touching
        
        // No damping when mostly open (pressure < 0.3)
        // Gentle damping when partially closed (0.3 - 0.7)
        // Strong damping when closing (> 0.7)
        
        if (pressure > 0.7f) {
            // Actively closing - strong damp
            modalBank.damp(pressure * 0.08f);
            wash.damp(pressure * 0.10f);
        } else if (pressure > 0.3f) {
            // Partial close - gentle damp (quadratic for smoothness)
            float dampAmt = (pressure - 0.3f) * (pressure - 0.3f) * 0.02f;
            modalBank.damp(dampAmt);
            wash.damp(dampAmt * 1.2f);
        }
        // Below 0.3 = no damping, let it ring!
        
        if (type == 2) {  // Hi-Hat mode switching
            if (pressure > 0.8f && currentHiHatOpen) {
                modalBank.configure(HIHAT_CLOSED_MODES, NUM_HIHAT_CLOSED_MODES, sizeMod, toneMod, decayMod);
                currentHiHatOpen = false;
            } else if (pressure < 0.2f && !currentHiHatOpen) {
                modalBank.configure(HIHAT_OPEN_MODES, NUM_HIHAT_OPEN_MODES, sizeMod, toneMod, decayMod);
                currentHiHatOpen = true;
            }
        }
    }
    
    float process() {
        float modal = modalBank.process();
        float transient = stick.process(rng);
        float washNoise = wash.process(rng);
        float output = modal + transient + washNoise;
        
        // Sizzle (rivets/chains) for Ride Sizzle style
        if (sizzleEnabled && sizzleLevel > 0.001f) {
            float sizzle = rng.nextBipolar() * sizzleLevel * 0.15f;
            output += sizzle;
            sizzleLevel *= 0.9997f;  // Slow decay
        }
        
        // Soft clipping
        if (output > 1.0f) output = 1.0f - 1.0f / (output + 1.0f);
        else if (output < -1.0f) output = -1.0f + 1.0f / (-output + 1.0f);
        
        return output;
    }
    
    void reset() { 
        modalBank.reset(); 
        stick.reset();
        wash.reset();
    }
    
private:
    float sampleRate = 48000.0f;
    int type = 0, strikeType = 0;
    float sizeMod = 0.5f, toneMod = 0.5f, decayMod = 0.5f;
    int style = 0;
    bool sizzleEnabled = false;
    float sizzleLevel = 0.0f;  // For envelope following
    float stickBrightness = 0.7f;
    bool currentHiHatOpen = true;
    
    ModalBank modalBank;
    StickTransient stick;
    WashNoise wash;
    FastRandom rng;
};

// ============================================================================
// SIMPLE REVERB
// ============================================================================

// ============================================================================
// FILTERS (from El Applauso)
// ============================================================================

class OnePole {
public:
    void setSampleRate(float sr) { sampleRate = sr; }
    void setFreq(float freq) {
        freq = clampf(freq, 20.0f, sampleRate * 0.49f);
        coef = expf(-TWO_PI * freq / sampleRate);
    }
    float processLP(float x) { return state = x * (1.0f - coef) + state * coef; }
    float processHP(float x) { return x - processLP(x); }
    void reset() { state = 0.0f; }
private:
    float sampleRate = 48000.0f, state = 0.0f, coef = 0.0f;
};

class SVFilter {
public:
    void setSampleRate(float sr) { sampleRate = sr; }
    void setParams(float freq, float q) {
        freq = clampf(freq, 20.0f, sampleRate * 0.45f);
        q = clampf(q, 0.5f, 20.0f);
        g = tanf(PI * freq / sampleRate);
        k = 1.0f / q;
    }
    float processBP(float x) {
        float hp = (x - (k + g) * s1 - s2) / (1.0f + k * g + g * g);
        float bp = g * hp + s1;
        float lp = g * bp + s2;
        s1 = g * hp + bp; s2 = g * bp + lp;
        return bp;
    }
    void reset() { s1 = s2 = 0.0f; }
private:
    float sampleRate = 48000.0f, g = 0.0f, k = 0.0f, s1 = 0.0f, s2 = 0.0f;
};

// ============================================================================
// FDN REVERB (from El Applauso v2.3.2)
// ============================================================================

struct VenueConfig {
    float rt60;
    float predelayMs;
    float hfDamping;
    float lfDamping;
    float diffusion;
    float erLevel;
    float lateLevel;
    float erSpacing;
    float modDepth;
    float modRate;
    float flutterAmount;
    float modalEmphasis;
    float airAbsorption;
};

static const VenueConfig venues[5] = {
    // Studio
    { 0.30f, 2.0f, 0.80f, 0.55f, 0.35f, 0.20f, 0.10f, 0.4f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f },
    // Garage
    { 1.80f, 8.0f, 0.15f, 0.25f, 0.20f, 0.85f, 0.55f, 1.4f, 0.02f, 0.8f, 0.65f, 0.25f, 0.0f },
    // Bar
    { 0.95f, 18.0f, 0.60f, 0.40f, 0.60f, 0.45f, 0.35f, 1.0f, 0.03f, 0.6f, 0.10f, 0.15f, 0.0f },
    // Basement
    { 2.20f, 4.0f, 0.35f, 0.12f, 0.30f, 0.80f, 0.50f, 0.5f, 0.01f, 0.4f, 0.35f, 0.75f, 0.0f },
    // Arena
    { 3.80f, 95.0f, 0.70f, 0.35f, 0.92f, 0.18f, 0.80f, 2.5f, 0.06f, 0.5f, 0.0f, 0.0f, 0.85f }
};

class FDNReverb {
public:
    void setSampleRate(float sr) {
        sampleRate = sr;
        
        tankLen[0] = (int)(sr * 0.0293f);
        tankLen[1] = (int)(sr * 0.0337f);
        tankLen[2] = (int)(sr * 0.0391f);
        tankLen[3] = (int)(sr * 0.0447f);
        
        for (int i = 0; i < 4; i++) {
            if (tankLen[i] >= TANK_SIZE) tankLen[i] = TANK_SIZE - 1;
            tankIdx[i] = 0;
        }
        
        apLen[0] = (int)(sr * 0.0047f);
        apLen[1] = (int)(sr * 0.0036f);
        apLen[2] = (int)(sr * 0.0127f);
        apLen[3] = (int)(sr * 0.0093f);
        for (int i = 0; i < 4; i++) {
            if (apLen[i] >= AP_SIZE) apLen[i] = AP_SIZE - 1;
            apIdx[i] = 0;
        }
        
        maxPredelay = (int)(sr * 0.150f);
        if (maxPredelay >= PREDELAY_SIZE) maxPredelay = PREDELAY_SIZE - 1;
        predelayIdx = 0;
        
        modalFreqs[0] = 68.0f;
        modalFreqs[1] = 101.0f;
        modalFreqs[2] = 164.0f;
        modalFreqs[3] = 272.0f;
        
        for (int i = 0; i < 4; i++) {
            modalFilters[i].setSampleRate(sr);
            modalFilters[i].setParams(modalFreqs[i], 8.0f + i * 2.0f);
        }
        
        flutterLen[0] = (int)(sr * 0.0175f);
        flutterLen[1] = (int)(sr * 0.0183f);
        for (int i = 0; i < 2; i++) {
            if (flutterLen[i] >= FLUTTER_SIZE) flutterLen[i] = FLUTTER_SIZE - 1;
            flutterIdx[i] = 0;
        }
        
        airAbsFilter[0].setSampleRate(sr);
        airAbsFilter[1].setSampleRate(sr);
        
        reset();
    }
    
    void setVenue(int venue, float amt) {
        if (venue < 0) venue = 0;
        if (venue > 4) venue = 4;
        currentVenue = venue;
        amount = amt;
        
        const VenueConfig& v = venues[venue];
        
        float avgDelay = (tankLen[0] + tankLen[1] + tankLen[2] + tankLen[3]) * 0.25f;
        float targetRT60 = v.rt60 * amt + 0.1f * (1.0f - amt);
        feedback = powf(0.001f, avgDelay / (targetRT60 * sampleRate));
        if (feedback > 0.985f) feedback = 0.985f;
        
        float hfDampFreq = 1500.0f + (1.0f - v.hfDamping) * 10000.0f;
        float lfDampFreq = 80.0f + v.lfDamping * 300.0f;
        
        float whf = 2.0f * PI * hfDampFreq / sampleRate;
        hfDampCoef = whf / (whf + 1.0f);
        
        float wlf = 2.0f * PI * lfDampFreq / sampleRate;
        lfDampCoef = 1.0f - (wlf / (wlf + 1.0f));
        
        predelaySamples = (int)(v.predelayMs * sampleRate / 1000.0f * amt);
        if (predelaySamples >= PREDELAY_SIZE) predelaySamples = PREDELAY_SIZE - 1;
        
        erLevel = v.erLevel * amt;
        lateLevel = v.lateLevel * amt;
        diffusion = v.diffusion;
        erSpacing = v.erSpacing;
        
        modDepth = v.modDepth;
        modRate = v.modRate;
        flutterAmount = v.flutterAmount;
        modalEmphasis = v.modalEmphasis;
        airAbsorption = v.airAbsorption;
        
        float airFreq = 8000.0f - airAbsorption * 5000.0f;
        airAbsFilter[0].setFreq(airFreq);
        airAbsFilter[1].setFreq(airFreq * 0.95f);
    }
    
    void process(float inL, float inR, float& outL, float& outR) {
        if (amount < 0.001f) {
            outL = 0.0f;
            outR = 0.0f;
            return;
        }
        
        float input = (inL + inR) * 0.5f;
        inputLP += 0.25f * (input - inputLP);
        input = inputLP;
        
        predelayBuf[predelayIdx] = input;
        int readIdx = (predelayIdx - predelaySamples + PREDELAY_SIZE) % PREDELAY_SIZE;
        float delayed = predelayBuf[readIdx];
        predelayIdx = (predelayIdx + 1) % PREDELAY_SIZE;
        
        float diffused = delayed;
        float apCoef1 = 0.75f * diffusion;
        float apCoef2 = 0.625f * diffusion;
        
        for (int i = 0; i < 4; i++) {
            float coef = (i < 2) ? apCoef1 : apCoef2;
            int rIdx = (apIdx[i] - apLen[i] + AP_SIZE) % AP_SIZE;
            float del = apBuf[i][rIdx];
            float temp = diffused + del * coef;
            apBuf[i][apIdx[i]] = temp;
            diffused = del - temp * coef;
            apIdx[i] = (apIdx[i] + 1) % AP_SIZE;
        }
        
        erBuf[erIdx] = diffused;
        float erL = 0.0f, erR = 0.0f;
        
        float erGains[6] = {0.8f, 0.7f, 0.55f, 0.45f, 0.35f, 0.25f};
        float erPans[6] = {-0.4f, 0.5f, -0.2f, 0.3f, -0.6f, 0.4f};
        
        for (int i = 0; i < 6; i++) {
            float tapMs = 3.0f + i * 8.0f * erSpacing;
            int tapSamples = (int)(tapMs * sampleRate / 1000.0f);
            if (tapSamples >= ER_SIZE) tapSamples = ER_SIZE - 1;
            
            int tapIdx = (erIdx - tapSamples + ER_SIZE) % ER_SIZE;
            float tap = erBuf[tapIdx] * erGains[i] * erLevel;
            float panR = (erPans[i] + 1.0f) * 0.5f;
            erL += tap * (1.0f - panR);
            erR += tap * panR;
        }
        erIdx = (erIdx + 1) % ER_SIZE;
        
        if (flutterAmount > 0.01f) {
            flutterBuf[0][flutterIdx[0]] = diffused + flutterBuf[0][(flutterIdx[0] - flutterLen[0] + FLUTTER_SIZE) % FLUTTER_SIZE] * 0.6f;
            float flutterL = flutterBuf[0][(flutterIdx[0] - flutterLen[0] + FLUTTER_SIZE) % FLUTTER_SIZE];
            flutterIdx[0] = (flutterIdx[0] + 1) % FLUTTER_SIZE;
            
            flutterBuf[1][flutterIdx[1]] = diffused + flutterBuf[1][(flutterIdx[1] - flutterLen[1] + FLUTTER_SIZE) % FLUTTER_SIZE] * 0.6f;
            float flutterR = flutterBuf[1][(flutterIdx[1] - flutterLen[1] + FLUTTER_SIZE) % FLUTTER_SIZE];
            flutterIdx[1] = (flutterIdx[1] + 1) % FLUTTER_SIZE;
            
            erL += flutterL * flutterAmount * 0.4f;
            erR += flutterR * flutterAmount * 0.4f;
        }
        
        if (modalEmphasis > 0.01f) {
            float modalSig = 0.0f;
            for (int i = 0; i < 4; i++) {
                modalSig += modalFilters[i].processBP(diffused) * (0.4f - i * 0.08f);
            }
            float modalMix = modalSig * modalEmphasis * 0.35f;
            erL += modalMix;
            erR += modalMix;
            diffused += modalMix * 0.5f;
        }
        
        modPhase += modRate * TWO_PI / sampleRate;
        if (modPhase > TWO_PI) modPhase -= TWO_PI;
        float modOffset = sinf(modPhase) * modDepth * 32.0f;
        
        float tankOut[4];
        for (int i = 0; i < 4; i++) {
            float modMult = (i & 1) ? 1.0f : -1.0f;
            float modLen = (float)tankLen[i] + modOffset * modMult;
            int intLen = (int)modLen;
            float frac = modLen - intLen;
            
            if (intLen < 1) intLen = 1;
            if (intLen >= TANK_SIZE - 1) intLen = TANK_SIZE - 2;
            
            int rIdx0 = (tankIdx[i] - intLen + TANK_SIZE) % TANK_SIZE;
            int rIdx1 = (tankIdx[i] - intLen - 1 + TANK_SIZE) % TANK_SIZE;
            
            float raw = tankBuf[i][rIdx0] * (1.0f - frac) + tankBuf[i][rIdx1] * frac;
            
            hfDampState[i] += hfDampCoef * (raw - hfDampState[i]);
            float hfDamped = hfDampState[i];
            
            lfDampState[i] = lfDampCoef * (raw + lfDampState[i] - lfPrevIn[i]);
            lfPrevIn[i] = raw;
            
            tankOut[i] = hfDamped - lfDampState[i] * (1.0f - lfDampCoef) * 0.5f;
        }
        
        float mixed[4];
        mixed[0] = 0.5f * (tankOut[0] + tankOut[1] + tankOut[2] + tankOut[3]);
        mixed[1] = 0.5f * (tankOut[0] - tankOut[1] + tankOut[2] - tankOut[3]);
        mixed[2] = 0.5f * (tankOut[0] + tankOut[1] - tankOut[2] - tankOut[3]);
        mixed[3] = 0.5f * (tankOut[0] - tankOut[1] - tankOut[2] + tankOut[3]);
        
        float inputScaled = diffused * 0.25f;
        for (int i = 0; i < 4; i++) {
            tankBuf[i][tankIdx[i]] = mixed[i] * feedback + inputScaled;
            tankIdx[i] = (tankIdx[i] + 1) % TANK_SIZE;
        }
        
        float lateL = (tankOut[0] + tankOut[2]) * 0.5f * lateLevel;
        float lateR = (tankOut[1] + tankOut[3]) * 0.5f * lateLevel;
        
        if (airAbsorption > 0.01f) {
            lateL = airAbsFilter[0].processLP(lateL);
            lateR = airAbsFilter[1].processLP(lateR);
        }
        
        outL = erL + lateL;
        outR = erR + lateR;
        
        float dcL = outL - dcInL + 0.995f * dcOutL;
        dcInL = outL; dcOutL = dcL; outL = dcL;
        
        float dcR = outR - dcInR + 0.995f * dcOutR;
        dcInR = outR; dcOutR = dcR; outR = dcR;
    }
    
    void reset() {
        memset(tankBuf, 0, sizeof(tankBuf));
        memset(apBuf, 0, sizeof(apBuf));
        memset(erBuf, 0, sizeof(erBuf));
        memset(predelayBuf, 0, sizeof(predelayBuf));
        memset(hfDampState, 0, sizeof(hfDampState));
        memset(lfDampState, 0, sizeof(lfDampState));
        memset(lfPrevIn, 0, sizeof(lfPrevIn));
        memset(flutterBuf, 0, sizeof(flutterBuf));
        for (int i = 0; i < 4; i++) { 
            tankIdx[i] = 0; 
            apIdx[i] = 0;
            modalFilters[i].reset();
        }
        for (int i = 0; i < 2; i++) {
            flutterIdx[i] = 0;
            airAbsFilter[i].reset();
        }
        erIdx = 0; predelayIdx = 0;
        inputLP = 0.0f;
        dcInL = dcInR = dcOutL = dcOutR = 0.0f;
        modPhase = 0.0f;
    }
    
private:
    static constexpr int TANK_SIZE = 2048;
    static constexpr int AP_SIZE = 640;
    static constexpr int ER_SIZE = 2048;
    static constexpr int PREDELAY_SIZE = 7200;
    static constexpr int FLUTTER_SIZE = 1024;
    
    float sampleRate = 48000.0f;
    float amount = 0.5f;
    int currentVenue = 0;
    
    float tankBuf[4][TANK_SIZE] = {};
    int tankLen[4] = {};
    int tankIdx[4] = {};
    float feedback = 0.5f;
    float hfDampCoef = 0.3f;
    float lfDampCoef = 0.1f;
    float hfDampState[4] = {};
    float lfDampState[4] = {};
    float lfPrevIn[4] = {};
    
    float modPhase = 0.0f;
    float modDepth = 0.0f;
    float modRate = 0.0f;
    
    float apBuf[4][AP_SIZE] = {};
    int apLen[4] = {};
    int apIdx[4] = {};
    float diffusion = 0.7f;
    
    float erBuf[ER_SIZE] = {};
    int erIdx = 0;
    float erLevel = 0.4f;
    float erSpacing = 1.0f;
    
    float predelayBuf[PREDELAY_SIZE] = {};
    int predelayIdx = 0;
    int predelaySamples = 0;
    int maxPredelay = 0;
    
    float flutterBuf[2][FLUTTER_SIZE] = {};
    int flutterLen[2] = {};
    int flutterIdx[2] = {};
    float flutterAmount = 0.0f;
    
    SVFilter modalFilters[4];
    float modalFreqs[4] = {};
    float modalEmphasis = 0.0f;
    
    OnePole airAbsFilter[2];
    float airAbsorption = 0.0f;
    
    float inputLP = 0.0f;
    float dcInL = 0.0f, dcInR = 0.0f;
    float dcOutL = 0.0f, dcOutR = 0.0f;
    
    float lateLevel = 0.4f;
};

// ============================================================================
// PARAMETERS
// ============================================================================

enum {
    kParamType,
    kParamStrike,
    kParamSize,
    kParamStyle,
    kParamTone,
    kParamDecay,
    kParamGain,
    
    kParamVenueEnable,
    kParamVenueType,
    kParamReverb,
    kParamPan,
    
    kParamStickInput,
    kParamChokeInput,
    kParamStickVel,
    kParamChokeVel,
    kParamOutputL,
    kParamOutputR,
    kParamOutputMode,
    
    kNumParams
};

static const char* const typeNames[] = { "Ride", "Crash", "Hi-Hat", "Splash", nullptr };
static const char* const strikeNames[] = { "Edge", "Bell", nullptr };
static const char* const sizeNames[] = { "Small", "Medium", "Large", nullptr };
static const char* const styleNames[] = { "Traditional", "Heavy", "Sizzle", nullptr };  // For Ride
static const char* const offOnNames[] = { "Off", "On", nullptr };
static const char* const venueTypeNames[] = { "Studio", "Garage", "Bar", "Basement", "Arena", nullptr };

static _NT_parameter parameters[] = {
    { .name = "Type", .min = 0, .max = 3, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = typeNames },
    { .name = "Strike", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = strikeNames },
    { .name = "Size", .min = 0, .max = 2, .def = 1, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = sizeNames },
    { .name = "Style", .min = 0, .max = 2, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = styleNames },
    { .name = "Tone", .min = 0, .max = 100, .def = 50, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = nullptr },
    { .name = "Decay", .min = 0, .max = 100, .def = 50, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = nullptr },
    { .name = "Gain", .min = 0, .max = 150, .def = 100, .unit = kNT_unitNone, .scaling = 0, .enumStrings = nullptr },
    { .name = "Venue", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = offOnNames },
    { .name = "Type", .min = 0, .max = 4, .def = 2, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = venueTypeNames },
    { .name = "Amount", .min = 0, .max = 100, .def = 30, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = nullptr },
    { .name = "Pan", .min = -50, .max = 50, .def = 0, .unit = kNT_unitNone, .scaling = 0, .enumStrings = nullptr },
    // min=0 allows "None" for optional inputs/outputs
    { .name = "Stick In", .min = 1, .max = 28, .def = 1, .unit = kNT_unitAudioInput, .scaling = 0, .enumStrings = nullptr },
    { .name = "Choke In", .min = 0, .max = 28, .def = 0, .unit = kNT_unitAudioInput, .scaling = 0, .enumStrings = nullptr },  // Default to None!
    { .name = "Stick Vel", .min = 0, .max = 100, .def = 80, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = nullptr },
    { .name = "Choke Vel", .min = 0, .max = 100, .def = 50, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = nullptr },
    { .name = "Output L", .min = 1, .max = 28, .def = 13, .unit = kNT_unitAudioOutput, .scaling = 0, .enumStrings = nullptr },
    { .name = "Output R", .min = 0, .max = 28, .def = 14, .unit = kNT_unitAudioOutput, .scaling = 0, .enumStrings = nullptr },
    { .name = "Output Mode", .min = 0, .max = 1, .def = 0, .unit = kNT_unitOutputMode, .scaling = 0, .enumStrings = nullptr },
};

static const uint8_t pageSound[] = { kParamType, kParamStrike, kParamSize, kParamStyle, kParamTone, kParamDecay, kParamGain };
static const uint8_t pageVenue[] = { kParamVenueEnable, kParamVenueType, kParamReverb, kParamPan };
static const uint8_t pageRouting[] = { kParamStickInput, kParamChokeInput, kParamStickVel, kParamChokeVel, kParamOutputL, kParamOutputR, kParamOutputMode };

static const _NT_parameterPage pages[] = {
    { .name = "Sound", .numParams = ARRAY_SIZE(pageSound), .params = pageSound },
    { .name = "Venue", .numParams = ARRAY_SIZE(pageVenue), .params = pageVenue },
    { .name = "Routing", .numParams = ARRAY_SIZE(pageRouting), .params = pageRouting },
};

static const _NT_parameterPages parameterPages = {
    .numPages = ARRAY_SIZE(pages),
    .pages = pages,
};

// ============================================================================
// DTC STRUCTURE
// ============================================================================

struct _losPlatosDTC {
    float sampleRate;
    bool lastStickTrig;
    float smoothedChoke;
    int lastType;      // Track last type for change detection
    int lastStrike;    // Track last strike for change detection
    int lastSize;      // Track last size for change detection
    CymbalEngine cymbal;
    FDNReverb reverb;
};

struct _losPlatosAlgorithm : public _NT_algorithm {
    _losPlatosAlgorithm(_losPlatosDTC* d) : dtc(d) {}
    _losPlatosDTC* dtc;
};

// ============================================================================
// API FUNCTIONS
// ============================================================================

void calculateRequirements(_NT_algorithmRequirements& req, const int32_t*) {
    req.numParameters = kNumParams;
    req.sram = sizeof(_losPlatosAlgorithm);
    req.dtc = sizeof(_losPlatosDTC);
}

_NT_algorithm* construct(const _NT_algorithmMemoryPtrs& ptrs, const _NT_algorithmRequirements& req, const int32_t*) {
    memset(ptrs.dtc, 0, sizeof(_losPlatosDTC));
    _losPlatosDTC* dtc = new (ptrs.dtc) _losPlatosDTC();
    _losPlatosAlgorithm* alg = new (ptrs.sram) _losPlatosAlgorithm(dtc);
    
    alg->parameters = parameters;
    alg->parameterPages = &parameterPages;
    
    dtc->sampleRate = (float)NT_globals.sampleRate;
    dtc->lastStickTrig = false;
    dtc->smoothedChoke = 0.0f;
    
    // Initialize type tracking with invalid values to force first configure
    dtc->lastType = -1;
    dtc->lastStrike = -1;
    dtc->lastSize = -1;
    
    dtc->cymbal.setSampleRate(dtc->sampleRate);
    dtc->reverb.setSampleRate(dtc->sampleRate);
    dtc->reverb.setVenue(0, 0.0f);  // Off by default
    
    // Note: Cymbal will be configured on first step() when type tracking detects change
    
    return alg;
}

void parameterChanged(_NT_algorithm* self, int p) {
    _losPlatosAlgorithm* alg = (_losPlatosAlgorithm*)self;
    _losPlatosDTC* dtc = alg->dtc;
    
    int algIdx = NT_algorithmIndex(self);
    int offset = NT_parameterOffset();
    
    if (p <= kParamGain) {
        int type = alg->v[kParamType];
        dtc->cymbal.configure(type, alg->v[kParamStrike], alg->v[kParamSize],
                              alg->v[kParamStyle], (float)alg->v[kParamTone], 
                              (float)alg->v[kParamDecay]);
        bool isRide = (type == 0);
        NT_setParameterGrayedOut(algIdx, kParamStrike + offset, !isRide);
        NT_setParameterGrayedOut(algIdx, kParamStyle + offset, !isRide);  // Style only for Ride
    }
    
    if (p >= kParamVenueEnable && p <= kParamPan) {
        bool venueEnabled = (alg->v[kParamVenueEnable] == 1);
        if (venueEnabled) {
            int venueType = alg->v[kParamVenueType];
            float amount = alg->v[kParamReverb] / 100.0f;
            dtc->reverb.setVenue(venueType, amount);
        } else {
            dtc->reverb.setVenue(0, 0.0f);  // Off
        }
        NT_setParameterGrayedOut(algIdx, kParamVenueType + offset, !venueEnabled);
        NT_setParameterGrayedOut(algIdx, kParamReverb + offset, !venueEnabled);
        NT_setParameterGrayedOut(algIdx, kParamPan + offset, !venueEnabled);
    }
}

void step(_NT_algorithm* self, float* busFrames, int numFramesBy4) {
    _losPlatosAlgorithm* alg = (_losPlatosAlgorithm*)self;
    _losPlatosDTC* dtc = alg->dtc;
    int numFrames = numFramesBy4 * 4;
    
    // SAFETY: Check if cymbal type changed and reconfigure if needed
    // This catches cases where parameterChanged wasn't called properly
    int curType = alg->v[kParamType];
    int curStrike = alg->v[kParamStrike];
    int curSize = alg->v[kParamSize];
    if (curType != dtc->lastType || curStrike != dtc->lastStrike || curSize != dtc->lastSize) {
        dtc->cymbal.configure(curType, curStrike, curSize,
                              alg->v[kParamStyle], (float)alg->v[kParamTone], 
                              (float)alg->v[kParamDecay]);
        dtc->lastType = curType;
        dtc->lastStrike = curStrike;
        dtc->lastSize = curSize;
    }
    
    const float* stick = busFrames + (alg->v[kParamStickInput] - 1) * numFrames;
    
    int chokeInput = alg->v[kParamChokeInput];
    const float* choke = (chokeInput > 0) ? busFrames + (chokeInput - 1) * numFrames : nullptr;
    
    float* outL = busFrames + (alg->v[kParamOutputL] - 1) * numFrames;
    
    int outputR = alg->v[kParamOutputR];
    float* outR = (outputR > 0) ? busFrames + (outputR - 1) * numFrames : nullptr;
    
    bool outputReplace = (alg->v[kParamOutputMode] == 1);
    bool venueEnabled = (alg->v[kParamVenueEnable] == 1);
    float stickVel = alg->v[kParamStickVel] / 100.0f;
    float chokeVel = alg->v[kParamChokeVel] / 100.0f;
    float gainParam = alg->v[kParamGain] / 100.0f;
    float pan = venueEnabled ? (alg->v[kParamPan] / 50.0f) : 0.0f;
    
    float panL = sqrtf((1.0f - pan) * 0.5f);
    float panR = sqrtf((1.0f + pan) * 0.5f);
    
    for (int i = 0; i < numFrames; i++) {
        bool trig = stick[i] > 0.5f;
        if (trig && !dtc->lastStickTrig) {
            float vel = clampf(stick[i] * stickVel, 0.1f, 1.0f);
            dtc->cymbal.trigger(vel);
        }
        dtc->lastStickTrig = trig;
        
        if (choke) {
            float chokeTarget = clampf(choke[i] * chokeVel, 0.0f, 1.0f);
            dtc->smoothedChoke += (chokeTarget - dtc->smoothedChoke) * 0.01f;
            dtc->cymbal.updateChoke(dtc->smoothedChoke);
        }
        
        float cymbalOut = dtc->cymbal.process() * gainParam;
        
        float sigL = cymbalOut * panL;
        float sigR = cymbalOut * panR;
        
        if (venueEnabled) {
            float wetL, wetR;
            dtc->reverb.process(cymbalOut, cymbalOut, wetL, wetR);
            sigL += wetL;
            sigR += wetR;
        }
        
        if (outputReplace) {
            outL[i] = sigL;
            if (outR) outR[i] = sigR;
        } else {
            outL[i] += sigL;
            if (outR) outR[i] += sigR;
        }
    }
}

// ============================================================================
// FACTORY
// ============================================================================

static const _NT_factory factory = {
    .guid = NT_MULTICHAR( 'L', 'o', 'P', 'l' ),
    .name = "Los Platos",
    .description = "Physical Modeling Cymbal Synth v0.1.1-dev",
    .numSpecifications = 0,
    .calculateRequirements = calculateRequirements,
    .construct = construct,
    .parameterChanged = parameterChanged,
    .step = step,
    .tags = kNT_tagInstrument,
};

uintptr_t pluginEntry(_NT_selector selector, uint32_t data) {
    switch (selector) {
    case kNT_selector_version: return kNT_apiVersionCurrent;
    case kNT_selector_numFactories: return 1;
    case kNT_selector_factoryInfo: return (uintptr_t)((data == 0) ? &factory : nullptr);
    }
    return 0;
}
