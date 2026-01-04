/*
 * Los Platos v3.4.0 - Realistic Cymbal Synthesizer
 * Expert Sleepers Disting NT
 *
 * CHANGES IN v3.4.0:
 * - Re-enabled stick transient at 0.25x (was disabled)
 * - Reduced strike energy to 1.5x (was 5x)
 * - Ride Bell: reduced low freq amplitudes, extended high freq decays
 * - High frequency modes need longer decay to be audible
 *
 * v3.3.0: Diagnostic build (broken - disabled transient)
 * v3.2.x: Fixed modes from measured samples
 * v3.1.x: Fixed Edge vs Bell reversal
 * v3.0.0: Complete rewrite using true modal synthesis
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
static const ModeData RIDE_EDGE_MODES[] = {
    { 47.0f,   0.45f, 40.0f },
    { 102.0f,  1.00f, 35.0f },
    { 172.0f,  0.40f, 30.0f },
    { 228.0f,  0.50f, 28.0f },
    { 307.0f,  0.25f, 25.0f },
    { 384.0f,  0.33f, 22.0f },
    { 451.0f,  0.33f, 20.0f },
    { 517.0f,  0.16f, 18.0f },
    { 567.0f,  0.35f, 16.0f },
    { 639.0f,  0.09f, 14.0f },
    { 698.0f,  0.15f, 12.0f },
    { 751.0f,  0.09f, 10.0f },
    { 803.0f,  0.08f, 8.0f },
    { 898.0f,  0.15f, 6.0f },
    { 1023.0f, 0.03f, 4.0f },
    { 1500.0f, 0.02f, 2.0f },
};
static constexpr int NUM_RIDE_EDGE_MODES = sizeof(RIDE_EDGE_MODES) / sizeof(ModeData);

// CRASH - Measured from real crash cymbal  
// Centroid: 2845 Hz, Decay: 8.30s
// Energy: <1kHz 67% | 1-5kHz 15% | >5kHz 18%
// Needs both low body AND high shimmer
static const ModeData CRASH_MODES[] = {
    {   123.0f, 0.45f, 8.0f },   // Low body
    {   265.0f, 1.00f, 7.0f },   // LOUDEST - fundamental crash
    {   383.0f, 0.70f, 6.0f },
    {   511.0f, 0.60f, 5.0f },   // From real!
    {   653.0f, 0.55f, 4.5f },   // From real!
    {   800.0f, 0.45f, 4.0f },
    {  1100.0f, 0.40f, 3.0f },
    {  1800.0f, 0.45f, 2.0f },
    {  2800.0f, 0.50f, 1.5f },   // Adding more highs!
    {  4000.0f, 0.45f, 1.0f },
    {  5500.0f, 0.40f, 0.7f },
    {  7000.0f, 0.35f, 0.5f },
    {  9000.0f, 0.30f, 0.3f },
};
static constexpr int NUM_CRASH_MODES = sizeof(CRASH_MODES) / sizeof(ModeData);

// HI-HAT OPEN - Measured from real hi-hat
// Centroid: 8407 Hz, Decay: 1.13s
// EXTREMELY BRIGHT - 72% energy above 5kHz!
// Increased decay times to match real ~1s sustain
static const ModeData HIHAT_OPEN_MODES[] = {
    {  4928.0f, 0.55f, 1.2f },
    {  4987.0f, 0.60f, 1.1f },
    {  5387.0f, 0.60f, 1.0f },
    {  5437.0f, 1.00f, 0.95f },  // LOUDEST
    {  5547.0f, 0.90f, 0.90f },
    {  6020.0f, 0.60f, 0.85f },
    {  6436.0f, 0.85f, 0.80f },  // Peak from real sample!
    {  6567.0f, 0.80f, 0.75f },
    {  6703.0f, 0.85f, 0.70f },
    {  7700.0f, 0.55f, 0.60f },
    {  8385.0f, 0.50f, 0.50f },
    {  9486.0f, 0.70f, 0.40f },  // Peak from real sample!
    { 11000.0f, 0.35f, 0.30f },
    { 13000.0f, 0.25f, 0.20f },
};
static constexpr int NUM_HIHAT_OPEN_MODES = sizeof(HIHAT_OPEN_MODES) / sizeof(ModeData);

// HI-HAT CLOSED - Similar brightness to open but VERY short decay
// Tight, sizzly, metallic - quick 15-40ms decay
static const ModeData HIHAT_CLOSED_MODES[] = {
    {  5000.0f, 0.50f, 0.035f },
    {  5500.0f, 0.80f, 0.030f },
    {  6000.0f, 1.00f, 0.028f },  // Peak
    {  6500.0f, 0.90f, 0.025f },
    {  7500.0f, 0.70f, 0.022f },
    {  8500.0f, 0.60f, 0.020f },
    { 10000.0f, 0.50f, 0.018f },
    { 12000.0f, 0.40f, 0.015f },
};
static constexpr int NUM_HIHAT_CLOSED_MODES = sizeof(HIHAT_CLOSED_MODES) / sizeof(ModeData);

// SPLASH - Measured from real splash cymbal
// Centroid: 10407 Hz, Decay: 0.63s
// EXTREMELY BRIGHT - 88% energy above 5kHz! The brightest cymbal.
// Peaks from real: 8528, 6396, 5806, 6196, 7230
static const ModeData SPLASH_MODES[] = {
    {  5806.0f, 0.65f, 0.70f },  // From real!
    {  6196.0f, 0.70f, 0.65f },  // From real!
    {  6396.0f, 0.85f, 0.60f },  // From real!
    {  7230.0f, 0.75f, 0.55f },  // From real!
    {  8528.0f, 1.00f, 0.50f },  // LOUDEST - from real!
    {  9285.0f, 0.50f, 0.45f },
    {  9817.0f, 0.75f, 0.40f },
    { 10270.0f, 0.55f, 0.35f },
    { 11500.0f, 0.60f, 0.30f },
    { 12848.0f, 0.70f, 0.25f },
    { 13502.0f, 0.35f, 0.20f },
    { 15000.0f, 0.25f, 0.15f },
};
static constexpr int NUM_SPLASH_MODES = sizeof(SPLASH_MODES) / sizeof(ModeData);

// ============================================================================
// MODAL BANK
// ============================================================================

static constexpr int MAX_MODES = 20;

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
        float freqMult = 0.8f + sizeMod * 0.4f;
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
        return sum / sqrtf((float)numModes + 1.0f);
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
        rng.seed(42);
    }
    
    void configure(int type, int strikeType, int size, int weight, float tone, float decay) {
        this->type = type;
        this->strikeType = strikeType;
        
        float sizeMod = (float)size / 2.0f;
        float toneMod = tone / 100.0f;
        float decayMod = decay / 100.0f;
        this->weightMod = (float)weight;
        
        switch (type) {
            case 0:
                if (strikeType == 1) {
                    modalBank.configure(RIDE_BELL_MODES, NUM_RIDE_BELL_MODES, sizeMod, toneMod, decayMod);
                    stickBrightness = 0.8f;
                } else {
                    modalBank.configure(RIDE_EDGE_MODES, NUM_RIDE_EDGE_MODES, sizeMod, toneMod, decayMod);
                    stickBrightness = 0.4f;
                }
                break;
            case 1:
                modalBank.configure(CRASH_MODES, NUM_CRASH_MODES, sizeMod, toneMod, decayMod);
                stickBrightness = 0.7f;
                break;
            case 2:
                currentHiHatOpen = true;
                modalBank.configure(HIHAT_OPEN_MODES, NUM_HIHAT_OPEN_MODES, sizeMod, toneMod, decayMod);
                stickBrightness = 0.95f;  // Very bright attack for hi-hat
                break;
            case 3:
                modalBank.configure(SPLASH_MODES, NUM_SPLASH_MODES, sizeMod, toneMod, decayMod);
                stickBrightness = 0.98f;  // Extremely bright attack for splash
                break;
        }
        
        this->sizeMod = sizeMod;
        this->toneMod = toneMod;
        this->decayMod = decayMod;
    }
    
    void trigger(float velocity) {
        modalBank.strike(velocity * (0.7f + weightMod * 0.6f));
        stick.trigger(velocity, stickBrightness);
    }
    
    void updateChoke(float pressure) {
        if (pressure > 0.05f) {
            modalBank.damp(pressure * 0.02f);
        }
        
        if (type == 2) {
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
        float output = modal + transient;
        
        if (output > 1.0f) output = 1.0f - 1.0f / (output + 1.0f);
        else if (output < -1.0f) output = -1.0f + 1.0f / (-output + 1.0f);
        
        return output;
    }
    
    void reset() { 
        modalBank.reset(); 
        stick.reset();
    }
    
private:
    float sampleRate = 48000.0f;
    int type = 0, strikeType = 0;
    float sizeMod = 0.5f, toneMod = 0.5f, decayMod = 0.5f, weightMod = 0.5f;
    float stickBrightness = 0.7f;
    bool currentHiHatOpen = true;
    
    ModalBank modalBank;
    StickTransient stick;
    FastRandom rng;
};

// ============================================================================
// SIMPLE REVERB
// ============================================================================

static constexpr int REVERB_BUFFER_SIZE = 4800;

class SimpleReverb {
public:
    void setSampleRate(float sr) {
        memset(buffer, 0, sizeof(buffer));
        writePos = 0;
    }
    
    void setParams(float size, float mix) {
        delayTime = (int)(size * REVERB_BUFFER_SIZE);
        if (delayTime < 1) delayTime = 1;
        if (delayTime >= REVERB_BUFFER_SIZE) delayTime = REVERB_BUFFER_SIZE - 1;
        wetMix = mix;
    }
    
    float process(float input) {
        int readPos = writePos - delayTime;
        if (readPos < 0) readPos += REVERB_BUFFER_SIZE;
        float delayed = buffer[readPos];
        float output = input + delayed * wetMix;
        buffer[writePos] = input + delayed * 0.4f;
        writePos = (writePos + 1) % REVERB_BUFFER_SIZE;
        return output;
    }
    
private:
    float buffer[REVERB_BUFFER_SIZE];
    int writePos = 0, delayTime = 1000;
    float wetMix = 0.0f;
};

// ============================================================================
// PARAMETERS
// ============================================================================

enum {
    kParamType,
    kParamStrike,
    kParamSize,
    kParamWeight,
    kParamTone,
    kParamDecay,
    kParamGain,
    
    kParamVenueEnable,
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
static const char* const weightNames[] = { "Thin", "Heavy", nullptr };
static const char* const offOnNames[] = { "Off", "On", nullptr };

static _NT_parameter parameters[] = {
    { .name = "Type", .min = 0, .max = 3, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = typeNames },
    { .name = "Strike", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = strikeNames },
    { .name = "Size", .min = 0, .max = 2, .def = 1, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = sizeNames },
    { .name = "Weight", .min = 0, .max = 1, .def = 1, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = weightNames },
    { .name = "Tone", .min = 0, .max = 100, .def = 50, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = nullptr },
    { .name = "Decay", .min = 0, .max = 100, .def = 50, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = nullptr },
    { .name = "Gain", .min = 0, .max = 150, .def = 100, .unit = kNT_unitNone, .scaling = 0, .enumStrings = nullptr },
    { .name = "Venue", .min = 0, .max = 1, .def = 0, .unit = kNT_unitEnum, .scaling = 0, .enumStrings = offOnNames },
    { .name = "Reverb", .min = 0, .max = 100, .def = 20, .unit = kNT_unitPercent, .scaling = 0, .enumStrings = nullptr },
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

static const uint8_t pageSound[] = { kParamType, kParamStrike, kParamSize, kParamWeight, kParamTone, kParamDecay, kParamGain };
static const uint8_t pageVenue[] = { kParamVenueEnable, kParamReverb, kParamPan };
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
    CymbalEngine cymbal;
    SimpleReverb reverb;
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
    
    dtc->cymbal.setSampleRate(dtc->sampleRate);
    dtc->cymbal.configure(0, 0, 1, 1, 50.0f, 50.0f);
    dtc->reverb.setSampleRate(dtc->sampleRate);
    dtc->reverb.setParams(0.0f, 0.0f);
    
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
                              alg->v[kParamWeight], (float)alg->v[kParamTone], 
                              (float)alg->v[kParamDecay]);
        bool isRide = (type == 0);
        NT_setParameterGrayedOut(algIdx, kParamStrike + offset, !isRide);
    }
    
    if (p >= kParamVenueEnable && p <= kParamPan) {
        bool venueEnabled = (alg->v[kParamVenueEnable] == 1);
        if (venueEnabled) {
            dtc->reverb.setParams(alg->v[kParamReverb] / 100.0f * 0.5f, 
                                  alg->v[kParamReverb] / 100.0f * 0.3f);
        } else {
            dtc->reverb.setParams(0.0f, 0.0f);
        }
        NT_setParameterGrayedOut(algIdx, kParamReverb + offset, !venueEnabled);
        NT_setParameterGrayedOut(algIdx, kParamPan + offset, !venueEnabled);
    }
}

void step(_NT_algorithm* self, float* busFrames, int numFramesBy4) {
    _losPlatosAlgorithm* alg = (_losPlatosAlgorithm*)self;
    _losPlatosDTC* dtc = alg->dtc;
    int numFrames = numFramesBy4 * 4;
    
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
            float wet = dtc->reverb.process(cymbalOut);
            sigL += wet * 0.3f;
            sigR += wet * 0.3f;
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
    .description = "Modal Cymbal Synthesizer v3.4.0",
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
