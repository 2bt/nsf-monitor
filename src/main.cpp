#include <cstdio>
#include <SDL2/SDL.h>
#include "record.hpp"

// https://wiki.nesdev.com/w/index.php/NSF
// TODO: bankswitching, PAL, ...


enum {
    MIXRATE           = 44100,
    APU_RATE          = 1789773,
    FRAMERATE         = 60,
    SAMPLES_PER_FRAME = MIXRATE / FRAMERATE,
};


struct {
    struct Env {
        bool is_const;
        bool loop;
        int  pos;
        int  vol;
        int  level;
        void start() {
            level = 15;
            pos   = -1;
        }
        void tick() {
            if (++pos >= vol + 1) {
                pos = 0;
                if (level > 0 && --level == 0 && loop) level = 15;
            }
        }
        float get_vol() const {
            return (is_const ? vol : level) / 15.0;
        }
    };

    struct {
        float pos;
        int   period;
        float pitch;
        float duty;
        Env   env;
        bool  sweep_enabled;
        bool  sweep_negate;
        int   sweep_shift;
        int   sweep_pos;
        int   sweep_period;
    } pulse[2];

    struct {
        float pos;
        float pitch;
        float vol;
    } tri;

    struct {
        uint16_t reg = 1;
        int      pos;
        int      period;
        Env      env;
    } noise;

} apu;


Record record;


void mix(float out[2]) {
    static size_t sample = 0;
    static size_t frame = 0;
    auto const& state = record.states[frame];
    if (sample == 0) {
        if (++frame > record.states.size()) frame = 0;

        // pulse
        for (int i = 0; i < 2; ++i) {
            auto& pulse = apu.pulse[i];
            if (state.is_set[i * 4]) {
                uint8_t value = state.reg[i * 4];
                static const float DUTY_TABLE[4] = { 0.125, 0.25, 0.5, 0.75 };
                pulse.duty         = DUTY_TABLE[value >> 6];
                pulse.env.vol      = value & 0x0f;
                pulse.env.is_const = value & 0x10;
                pulse.env.loop     = value & 0x20;
            }
            if (state.is_set[1 + i * 4]) {
                uint8_t value = state.reg[1 + i * 4];
                pulse.sweep_enabled = value & 0x80;
                pulse.sweep_period  = ((value >> 4) & 0x7) + 1;
                pulse.sweep_shift   = value & 0x7;
                pulse.sweep_negate  = value & 0x8;
            }
            if (state.is_set[2 + i * 4]) {
                uint8_t value = state.reg[2 + i * 4];
                pulse.period &= ~0xff;
                pulse.period |= value;
                pulse.pitch = APU_RATE / float(16 * (pulse.period + 1)) / MIXRATE;
            }
            if (state.is_set[3 + i * 4]) {
                uint8_t value = state.reg[3 + i * 4];
                pulse.period &= ~0xff00;
                pulse.period |= (value & 0x7) << 8;
                pulse.pitch = APU_RATE / float(16 * (pulse.period + 1)) / MIXRATE;
                pulse.env.start();
            }
        }

        // triangle
        {
            auto& tri = apu.tri;
            int   period = state.reg[0xa] | ((state.reg[0xb] & 0x7) << 8);
            tri.pitch    = APU_RATE / float(32 * (period + 1)) / MIXRATE;
            tri.vol      = state.reg[0x8] & 0x7f ? 1 : 0;
        }

        // noise
        {
            static const int PERIOD_TABLE[16] = { 
                4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
            };
            auto& noise = apu.noise;
            noise.period       = PERIOD_TABLE[state.reg[0xe] % 0xf];
            noise.env.vol      = state.reg[0xc] & 0x0f;
            noise.env.is_const = state.reg[0xc] & 0x10;
            noise.env.loop     = state.reg[0xc] & 0x20;
            if (state.is_set[0xf]) apu.noise.env.start();
        }
    }
    // quater frame envelope
    if (sample % (SAMPLES_PER_FRAME / 4) == 0) {
        apu.pulse[0].env.tick();
        apu.pulse[1].env.tick();
        apu.noise.env.tick();
    }
    // half frame sweep
    if (sample % (SAMPLES_PER_FRAME / 2) == 0) {
        for (int i = 0; i < 2; ++i) {
            auto& pulse = apu.pulse[i];
            if (!pulse.sweep_enabled) continue;
            if (++pulse.sweep_pos >= pulse.sweep_period) {
                pulse.sweep_pos = 0;
                int s = pulse.period >> pulse.sweep_shift;
                pulse.period += pulse.sweep_negate ? -s : s;
                pulse.pitch = APU_RATE / float(16 * (pulse.period + 1)) / MIXRATE;
            }
        }
    }
    if (++sample >= SAMPLES_PER_FRAME) sample = 0;

    out[0] = out[1] = 0;

    // pulse
    for (int i = 0; i < 2; ++i) {
        apu.pulse[i].pos += apu.pulse[i].pitch;
        apu.pulse[i].pos -= (int) apu.pulse[i].pos;

        float amp = apu.pulse[i].pos < apu.pulse[i].duty ? -1 : 1;
        amp *= apu.pulse[i].env.get_vol();

        // have some stereo
        out[0] += amp * (i ? 0.8 : 1.2);
        out[1] += amp * (i ? 1.2 : 0.8);
    }

    // triangle
    {
        apu.tri.pos += apu.tri.pitch;
        apu.tri.pos -= (int) apu.tri.pos;
        float amp = apu.tri.pos < 0.5 ? 1 - apu.tri.pos * 2: (apu.tri.pos - 0.5) * 2 - 1;
        amp *= apu.tri.vol;
        out[0] += amp;
        out[1] += amp;
    }

    // noise
    {
        for (int i = 0; i < 10; ++i) {
            apu.noise.pos += 4;
            if (++apu.noise.pos >= apu.noise.period) {
                apu.noise.pos -= apu.noise.period;
                int b = apu.noise.reg ^ (apu.noise.reg >> (state.reg[0xb] & 0x80 ? 6 : 1));
                apu.noise.reg = (apu.noise.reg >> 1) | ((b & 1) << 14);
            }
        }
        float amp = apu.noise.reg / float(1 << 14) - 1;
        amp *= apu.noise.env.get_vol() * 2.5;
        out[0] += amp;
        out[1] += amp;
    }
}

template <class T>
T clamp(T const& v, T const& min=0, T const& max=1) {
    return std::max(min, std::min(max, v));
}

void callback(void* u, Uint8* stream, int len) {
    short* buf = (short*) stream;
    for (; len > 0; len -= 4) {
        float f[2];
        mix(f);
        *buf++ = clamp<int>(f[0] * 7000, -32768, 32767);
        *buf++ = clamp<int>(f[1] * 7000, -32768, 32767);
    }
}


int main(int argc, char** argv) {
    if (argc != 2 && argc != 3) {
        printf("usage: %s nsf-file [song-number]\n", argv[0]);
        return 0;
    }

    if (!record.load(argv[1], argc == 3 ? atoi(argv[2]) : -1)) {
        printf("error: could not open file\n");
        return 1;
    }


    SDL_AudioSpec spec = { MIXRATE, AUDIO_S16, 2, 0, 1024, 0, 0, &callback };
    SDL_OpenAudio(&spec, nullptr);
    SDL_PauseAudio(0);
    printf("playing...\n");
    getchar();
    printf("done.\n");
    SDL_Quit();

    return 0;
}
