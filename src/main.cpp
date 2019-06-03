#include <cstdio>
#include <SDL2/SDL.h>
#include "record.hpp"
#include "fx.hpp"

// https://wiki.nesdev.com/w/index.php/NSF
// TODO: bankswitching, length counter, PAL, ...


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
        float speed;
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
        float speed;
        float vol;
        bool  gate;
    } tri;

    struct {
        uint16_t reg = 1;
        bool     mode;
        int      pos;
        int      period;
        Env      env;
    } noise;

} apu;


Record record;

bool active[4] = { 1, 1, 1, 1 };
bool playing   = true;
int  frame;


template <class T>
T clamp(T const& v, T const& min, T const& max) {
    return std::max(min, std::min(max, v));
}


void tick() {
    const Uint8* ks = SDL_GetKeyboardState(nullptr);
    bool ctrl = ks[SDL_SCANCODE_LCTRL] | ks[SDL_SCANCODE_RCTRL];
    if (ctrl) return;

    bool shift = ks[SDL_SCANCODE_LSHIFT] | ks[SDL_SCANCODE_RSHIFT];
    int speed = shift ? 5 : 1;

    if (ks[SDL_SCANCODE_LEFT])  frame -= speed;
    else if (ks[SDL_SCANCODE_RIGHT]) frame += speed;
    else if (playing) ++frame;
    frame = clamp<int>(frame, 0, record.states.size());
}


void mix(float out[2]) {
    static size_t sample = 0;
    if (sample == 0) {
        auto const& state = record.states[frame];
        tick();

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
            if (state.is_set[2 + i * 4] || state.is_set[3 + i * 4]) {
                pulse.period = state.reg[2 + i * 4] | ((state.reg[3 + i * 4] & 0x7) << 8);
                pulse.speed  = APU_RATE / float(16 * (pulse.period + 1)) / MIXRATE;
            }

            if (state.is_set[3 + i * 4]) pulse.env.start();
        }

        // triangle
        {
            auto& tri = apu.tri;
            int   period = state.reg[0xa] | ((state.reg[0xb] & 0x7) << 8);
            tri.speed    = APU_RATE / float(32 * (period + 1)) / MIXRATE;
            tri.gate     = state.reg[0x8] & 0x7f;
            if (period < 2) tri.gate = false;
        }

        // noise
        {
            static const int PERIOD_TABLE[16] = {
                4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068
            };
            auto& noise = apu.noise;
            noise.period       = PERIOD_TABLE[state.reg[0xe] & 0xf];
            noise.mode         = state.reg[0xe] & 0x80;
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
                pulse.speed = APU_RATE / float(16 * (pulse.period + 1)) / MIXRATE;
            }
        }
    }
    if (++sample >= SAMPLES_PER_FRAME) sample = 0;

    out[0] = out[1] = 0;

    // pulse
    for (int i = 0; i < 2; ++i) {
        if (apu.pulse[i].period < 8) continue;
        apu.pulse[i].pos += apu.pulse[i].speed;
        apu.pulse[i].pos -= (int) apu.pulse[i].pos;

        float amp = apu.pulse[i].pos < apu.pulse[i].duty ? -1 : 1;
        amp *= apu.pulse[i].env.get_vol();

        // have some stereo
        if (active[i]) {
            out[0] += amp * (i ? 0.8 : 1.2);
            out[1] += amp * (i ? 1.2 : 0.8);
        }
    }

    // triangle
    {
        apu.tri.pos += apu.tri.speed;
        apu.tri.pos -= (int) apu.tri.pos;
        float p = std::floor(apu.tri.pos * 32) / 32;
        float amp = p < 0.5 ? 1 - p * 4: (p - 0.5) * 4 - 1;
        if (apu.tri.gate) apu.tri.vol = std::min<float>(1, apu.tri.vol + 0.01);
        else              apu.tri.vol = std::max<float>(0, apu.tri.vol - 0.01);
        amp *= apu.tri.vol * 1.2;
        if (active[2]) {
            out[0] += amp;
            out[1] += amp;
        }
    }

    // noise
    {
        // NOTE: this is hacky
        // 10 * 4 == 40 == APU_RATE / MIXRATE
        for (int i = 0; i < 10; ++i) {
            apu.noise.pos += 4;
            if (++apu.noise.pos >= apu.noise.period) {
                apu.noise.pos -= apu.noise.period;
                int b = apu.noise.reg ^ (apu.noise.reg >> (apu.noise.mode ? 6 : 1));
                apu.noise.reg = (apu.noise.reg >> 1) | ((b & 1) << 14);
            }
        }
        float amp = apu.noise.reg / float(1 << 14) - 1;
        amp *= apu.noise.env.get_vol() * 2;
        if (active[3]) {
            out[0] += amp;
            out[1] += amp;
        }
    }
}

void audio_callback(void* u, Uint8* stream, int len) {
    short* buf = (short*) stream;
    for (; len > 0; len -= 4) {
        float f[2];
        mix(f);
        *buf++ = clamp<int>(f[0] * 7000, -32768, 32767);
        *buf++ = clamp<int>(f[1] * 7000, -32768, 32767);
    }
}


struct App : fx::App {

    int scale_x = 4;
    int scale_y = 6;
    int offset  = 0;
    int bar     = 24;

    void init() override {
        //SDL_AudioSpec spec = { MIXRATE, AUDIO_S16, 2, 0, 1024, 0, 0, &audio_callback };
        SDL_AudioSpec spec = { MIXRATE, AUDIO_S16, 2, 0, SAMPLES_PER_FRAME, 0, 0, &audio_callback };
        SDL_OpenAudio(&spec, nullptr);
        SDL_PauseAudio(0);
    }

    const char* title() const { return "nsf-monitor"; }

    void key(int code) override {
        const Uint8* ks = SDL_GetKeyboardState(nullptr);
        bool ctrl = ks[SDL_SCANCODE_LCTRL] | ks[SDL_SCANCODE_RCTRL];

        switch (code) {
        case SDL_SCANCODE_SPACE: playing ^= 1; break;
        case SDL_SCANCODE_LEFT: if (ctrl) frame = std::max(0, frame - 1); break;
        case SDL_SCANCODE_RIGHT: if (ctrl) ++frame; break;
        case SDL_SCANCODE_BACKSPACE: frame = 0; break;

        case SDL_SCANCODE_PAGEDOWN: scale_y = std::max(0, scale_y - 1); break;
        case SDL_SCANCODE_PAGEUP: ++scale_y; break;

        case SDL_SCANCODE_W: ++bar; break;
        case SDL_SCANCODE_S: --bar; break;
        case SDL_SCANCODE_D: ++offset; break;
        case SDL_SCANCODE_A: --offset; break;

        case SDL_SCANCODE_1: active[0] ^= 1; break;
        case SDL_SCANCODE_2: active[1] ^= 1; break;
        case SDL_SCANCODE_3: active[2] ^= 1; break;
        case SDL_SCANCODE_4: active[3] ^= 1; break;

        default: break;
        }
    }

    void textinput(char const* text) override {
        if (text[0] == '+') ++scale_x;
        if (text[0] == '-') scale_x = std::max(1, scale_x - 1);
    }

    void update() override {
        fx::set_color(0, 0, 0);
        fx::clear();

        int f = frame;
        int frames_per_screen = fx::screen_width() / scale_x;
        int start_frame = std::max(0, f - frames_per_screen / 2);

        // bars
        fx::set_color(50, 50, 50);
        for (int t = -((start_frame - offset) % bar); t < frames_per_screen; t += bar) {
             fx::draw_rectangle(true, t * scale_x, 0, 1, fx::screen_height());
        }



        for (int p = -48; p < 50; ++p) {

            int c = "101201011010"[(p + 120) % 12] - '0';
            c = 10 + c * 20;
            fx::set_color(c, c, c);

            int y = -p * scale_y + fx::screen_height() / 2;
            fx::draw_rectangle(true, 0, y, fx::screen_width(), 1 + scale_y - 2);

        }

         for (int n = start_frame; n < start_frame + frames_per_screen; ++n) {
            if (n >= (int) record.states.size()) break;
            auto const& state = record.states[n];

            int x = (n - start_frame) * scale_x;

            for (int i = 0; i < 3; ++i) {
                if (!active[i]) continue;

                int   period = state.reg[2 + i * 4] | ((state.reg[3 + i * 4] & 0x7) << 8);
                float speed  = APU_RATE / float(16 * (period + 1)) / 440;
                float pitch  = std::log2(speed) * 12;
                if (i == 2) pitch -= 12;

                int vol;
                if (i < 2) vol = state.reg[i * 4] & 0x0f;
                else       vol = state.reg[0x8] & 0x7f ? 0xa : 0;

                int v = 255 * std::pow(vol / 15.0, 0.5);
                if (i == 0) fx::set_color(v, v/3, v/3);
                if (i == 1) fx::set_color(v/3, v, v/3);
                if (i == 2) fx::set_color(v/3, v/3, v);

                int y = -pitch * scale_y + fx::screen_height() / 2;
                fx::draw_rectangle(true, x, y, scale_x, 1 + scale_y - 2);

                // sweep
                if (i < 2 && (state.is_set[1 + i * 4] | state.is_set[2 + i * 4] | state.is_set[3 + i * 4])) {
                    uint8_t value = state.reg[1 + i * 4];
                    bool sweep_enabled = value & 0x80;
                    bool sweep_negate  = value & 0x08;
                    if (sweep_enabled) {
                        if (sweep_negate) fx::draw_line(x, y - scale_y, x, y + scale_y);
                        else              fx::draw_line(x, y, x, y + scale_y * 2);
                    }
                }
            }

            // noise
            if (active[3]) {
                int  period = state.reg[0xe] & 0x0f;
                bool mode   = state.reg[0xe] & 0x80;
                int  vol    = state.reg[0xc] & 0x0f;
                bool is_const = state.reg[0xc] & 0x10;
                if (!is_const) {
                    if (state.is_set[0xf]) vol = 0xf;
                }
                int  v      = 200 * std::pow(vol / 15.0, 0.5);
                if (mode) fx::set_color(v / 3, v, v);
                else      fx::set_color(v, v, v);

                int y = (period - 49) * scale_y + fx::screen_height() / 2;
                fx::draw_rectangle(true, x, y, scale_x, 1 + scale_y - 2);
            }

         }

        // cursor
        fx::set_color(255, 255, 255);
        fx::draw_rectangle(true, (f - start_frame) * scale_x, 0, 1, fx::screen_height());

        // state
        auto const& state = record.states[f];
        for (int i = 0; i < 16; ++i) {
            if (state.is_set[i]) fx::set_font_color(250, 250, 250);
            else                 fx::set_font_color(150, 150, 150);
            fx::printf(i % 4 * 48 + 8, fx::screen_height() - (4 - i / 4) * 24, "%02X", state.reg[i]);
        }

        fx::set_font_color(250, 250, 250);
        fx::printf(8, 8, "%s - %d/%d", record.song_name.c_str(), record.song_nr, record.song_count);
        fx::printf(fx::screen_width() - 8 - 15 * 15, 8,      "position: %4d", f);
        fx::printf(fx::screen_width() - 8 - 15 * 15, 8 + 24, "     bar: %4d", bar);

    }
};


int main(int argc, char** argv) {
    if (argc != 2 && argc != 3) {
        printf("usage: %s nsf-file [song-number]\n", argv[0]);
        return 0;
    }

    if (!record.load(argv[1], argc == 3 ? atoi(argv[2]) : -1)) {
        return 1;
    }

    App app;
    return fx::run(app);
}
