#include <SDL2/SDL.h>
#include <vector>
#include "fx.hpp"

namespace fx {
namespace {

const uint8_t FONT[] = {
    0b00000000, 0b00011000, 0b01101100, 0b01101100, 0b00110000, 0b00000000, 0b00111000, 0b01100000, 0b00011000, 0b01100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000110,
    0b00000000, 0b00111100, 0b01101100, 0b01101100, 0b01111100, 0b11000110, 0b01101100, 0b01100000, 0b00110000, 0b00110000, 0b01100110, 0b00110000, 0b00000000, 0b00000000, 0b00000000, 0b00001100,
    0b00000000, 0b00111100, 0b01101100, 0b11111110, 0b11000000, 0b11001100, 0b00111000, 0b11000000, 0b01100000, 0b00011000, 0b00111100, 0b00110000, 0b00000000, 0b00000000, 0b00000000, 0b00011000,
    0b00000000, 0b00111100, 0b00000000, 0b01101100, 0b01111000, 0b00011000, 0b01110110, 0b00000000, 0b01100000, 0b00011000, 0b11111111, 0b11111100, 0b00000000, 0b11111100, 0b00000000, 0b00110000,
    0b00000000, 0b00011000, 0b00000000, 0b11111110, 0b00001100, 0b00110000, 0b11011100, 0b00000000, 0b01100000, 0b00011000, 0b00111100, 0b00110000, 0b00000000, 0b00000000, 0b00000000, 0b01100000,
    0b00000000, 0b00011000, 0b00000000, 0b01101100, 0b11111000, 0b01100110, 0b11001100, 0b00000000, 0b00110000, 0b00110000, 0b01100110, 0b00110000, 0b00110000, 0b00000000, 0b00110000, 0b11000000,
    0b00000000, 0b00000000, 0b00000000, 0b01101100, 0b00110000, 0b11000110, 0b01110110, 0b00000000, 0b00011000, 0b01100000, 0b00000000, 0b00000000, 0b00110000, 0b00000000, 0b00110000, 0b10000000,
    0b00000000, 0b00011000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01100000, 0b00000000, 0b00000000, 0b00000000,

    0b00111000, 0b00011000, 0b01111100, 0b01111110, 0b00011100, 0b11111100, 0b00111100, 0b11111110, 0b01111100, 0b01111100, 0b00000000, 0b00000000, 0b00011000, 0b00000000, 0b01100000, 0b01111000,
    0b01001100, 0b00111000, 0b11000110, 0b00001100, 0b00111100, 0b11000000, 0b01100000, 0b11000110, 0b11000110, 0b11000110, 0b00110000, 0b00110000, 0b00110000, 0b00000000, 0b00110000, 0b11001100,
    0b11000110, 0b00011000, 0b00001110, 0b00011000, 0b01101100, 0b11111100, 0b11000000, 0b00001100, 0b11000110, 0b11000110, 0b00110000, 0b00110000, 0b01100000, 0b11111100, 0b00011000, 0b00001100,
    0b11000110, 0b00011000, 0b00111100, 0b00111100, 0b11001100, 0b00000110, 0b11111100, 0b00011000, 0b01111100, 0b01111110, 0b00000000, 0b00000000, 0b11000000, 0b00000000, 0b00001100, 0b00011000,
    0b11000110, 0b00011000, 0b01111000, 0b00000110, 0b11111110, 0b00000110, 0b11000110, 0b00110000, 0b11000110, 0b00000110, 0b00000000, 0b00000000, 0b01100000, 0b00000000, 0b00011000, 0b00110000,
    0b01100100, 0b00011000, 0b11100000, 0b11000110, 0b00001100, 0b11000110, 0b11000110, 0b00110000, 0b11000110, 0b00001100, 0b00110000, 0b00110000, 0b00110000, 0b11111100, 0b00110000, 0b00000000,
    0b00111000, 0b01111110, 0b11111110, 0b01111100, 0b00001100, 0b01111100, 0b01111100, 0b00110000, 0b01111100, 0b01111000, 0b00110000, 0b00110000, 0b00011000, 0b00000000, 0b01100000, 0b00110000,
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,

    0b01111100, 0b00111000, 0b11111100, 0b00111100, 0b11111000, 0b11111110, 0b11111110, 0b00111110, 0b11000110, 0b01111110, 0b00011110, 0b11000110, 0b01100000, 0b11000110, 0b11000110, 0b01111100,
    0b11000110, 0b01101100, 0b11000110, 0b01100110, 0b11001100, 0b11000000, 0b11000000, 0b01100000, 0b11000110, 0b00011000, 0b00000110, 0b11001100, 0b01100000, 0b11101110, 0b11100110, 0b11000110,
    0b11011110, 0b11000110, 0b11000110, 0b11000000, 0b11000110, 0b11000000, 0b11000000, 0b11000000, 0b11000110, 0b00011000, 0b00000110, 0b11011000, 0b01100000, 0b11111110, 0b11110110, 0b11000110,
    0b11011110, 0b11000110, 0b11111100, 0b11000000, 0b11000110, 0b11111100, 0b11111100, 0b11001110, 0b11111110, 0b00011000, 0b00000110, 0b11110000, 0b01100000, 0b11111110, 0b11111110, 0b11000110,
    0b11011110, 0b11111110, 0b11000110, 0b11000000, 0b11000110, 0b11000000, 0b11000000, 0b11000110, 0b11000110, 0b00011000, 0b11000110, 0b11111000, 0b01100000, 0b11010110, 0b11011110, 0b11000110,
    0b11000000, 0b11000110, 0b11000110, 0b01100110, 0b11001100, 0b11000000, 0b11000000, 0b01100110, 0b11000110, 0b00011000, 0b11000110, 0b11011100, 0b01100000, 0b11000110, 0b11001110, 0b11000110,
    0b01111000, 0b11000110, 0b11111100, 0b00111100, 0b11111000, 0b11111110, 0b11000000, 0b00111110, 0b11000110, 0b01111110, 0b01111100, 0b11001110, 0b01111110, 0b11000110, 0b11000110, 0b01111100,
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,

    0b11111100, 0b01111100, 0b11111100, 0b01111000, 0b01111110, 0b11000110, 0b11000110, 0b11000110, 0b11000110, 0b01100110, 0b11111110, 0b01111000, 0b11000000, 0b01111000, 0b00010000, 0b00000000,
    0b11000110, 0b11000110, 0b11000110, 0b11001100, 0b00011000, 0b11000110, 0b11000110, 0b11000110, 0b11101110, 0b01100110, 0b00001110, 0b01100000, 0b01100000, 0b00011000, 0b00111000, 0b00000000,
    0b11000110, 0b11000110, 0b11000110, 0b11000000, 0b00011000, 0b11000110, 0b11000110, 0b11010110, 0b01111100, 0b01100110, 0b00011100, 0b01100000, 0b00110000, 0b00011000, 0b01101100, 0b00000000,
    0b11000110, 0b11000110, 0b11001110, 0b01111100, 0b00011000, 0b11000110, 0b11101110, 0b11111110, 0b00111000, 0b00111100, 0b00111000, 0b01100000, 0b00011000, 0b00011000, 0b11000110, 0b00000000,
    0b11111100, 0b11011110, 0b11111000, 0b00000110, 0b00011000, 0b11000110, 0b01111100, 0b11111110, 0b01111100, 0b00011000, 0b01110000, 0b01100000, 0b00001100, 0b00011000, 0b00000000, 0b00000000,
    0b11000000, 0b11001100, 0b11011100, 0b11000110, 0b00011000, 0b11000110, 0b00111000, 0b11101110, 0b11101110, 0b00011000, 0b11100000, 0b01100000, 0b00000110, 0b00011000, 0b00000000, 0b00000000,
    0b11000000, 0b01111010, 0b11001110, 0b01111100, 0b00011000, 0b01111100, 0b00010000, 0b11000110, 0b11000110, 0b00011000, 0b11111110, 0b01111000, 0b00000010, 0b01111000, 0b00000000, 0b00000000,
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b11111111,

    0b00110000, 0b00000000, 0b01100000, 0b00000000, 0b00000110, 0b00000000, 0b00001110, 0b00000000, 0b01100000, 0b00000000, 0b00000000, 0b01100000, 0b00011000, 0b00000000, 0b00000000, 0b00000000,
    0b00110000, 0b00000000, 0b01100000, 0b00000000, 0b00000110, 0b00000000, 0b00011000, 0b00000000, 0b01100000, 0b00011000, 0b00000110, 0b01100000, 0b00011000, 0b00000000, 0b00000000, 0b00000000,
    0b00011000, 0b00111100, 0b01111100, 0b00111110, 0b00111110, 0b00111100, 0b00011000, 0b00111110, 0b01100000, 0b00000000, 0b00000000, 0b01100010, 0b00011000, 0b01110110, 0b01111100, 0b00111100,
    0b00000000, 0b01100110, 0b01100110, 0b01100000, 0b01100110, 0b01100110, 0b01111110, 0b01100110, 0b01111100, 0b00011000, 0b00000110, 0b01100100, 0b00011000, 0b01101011, 0b01100110, 0b01100110,
    0b00000000, 0b01100110, 0b01100110, 0b01100000, 0b01100110, 0b01111110, 0b00011000, 0b01100110, 0b01100110, 0b00011000, 0b00000110, 0b01101000, 0b00011000, 0b01101011, 0b01100110, 0b01100110,
    0b00000000, 0b01100110, 0b01100110, 0b01100000, 0b01100110, 0b01100000, 0b00011000, 0b00111110, 0b01100110, 0b00011000, 0b00000110, 0b01111100, 0b00011000, 0b01101011, 0b01100110, 0b01100110,
    0b00000000, 0b00111011, 0b01111100, 0b00111110, 0b00111110, 0b00111110, 0b00011000, 0b00000110, 0b01100110, 0b00011000, 0b01100110, 0b01100110, 0b00011000, 0b01101011, 0b01100110, 0b00111100,
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00111100, 0b00000000, 0b00000000, 0b00111100, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,

    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00011100, 0b00011000, 0b11100000, 0b01110110, 0b00000000,
    0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00110000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00110000, 0b00011000, 0b00110000, 0b11011100, 0b00010000,
    0b01111100, 0b00111110, 0b01101110, 0b00111100, 0b11111100, 0b01100110, 0b01100110, 0b01100011, 0b01100011, 0b01100110, 0b01111110, 0b00110000, 0b00011000, 0b00110000, 0b00000000, 0b00111000,
    0b01100110, 0b01100110, 0b01110000, 0b01000000, 0b00110000, 0b01100110, 0b01100110, 0b01101011, 0b00110110, 0b01100110, 0b00001100, 0b11100000, 0b00000000, 0b00011100, 0b00000000, 0b01101100,
    0b01100110, 0b01100110, 0b01100000, 0b00111100, 0b00110000, 0b01100110, 0b01100110, 0b01101011, 0b00011100, 0b00101100, 0b00011000, 0b00110000, 0b00011000, 0b00110000, 0b00000000, 0b11000110,
    0b01111100, 0b00111110, 0b01100000, 0b00000110, 0b00110000, 0b01100110, 0b00100100, 0b01101011, 0b00110110, 0b00011000, 0b00110000, 0b00110000, 0b00011000, 0b00110000, 0b00000000, 0b11000110,
    0b01100000, 0b00000110, 0b01100000, 0b01111100, 0b00011100, 0b00111100, 0b00011000, 0b00110110, 0b01100011, 0b00110000, 0b01111110, 0b00011100, 0b00011000, 0b11100000, 0b00000000, 0b11111110,
    0b01100000, 0b00000110, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b01100000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000, 0b00000000,
};


SDL_Window*   m_window;
SDL_Renderer* m_renderer;
SDL_Texture*  m_font_tex;
bool          m_running;
int           m_screen_width  = 800;
int           m_screen_height = 600;

} // namespace


int run(App& app) {
    SDL_Init(SDL_INIT_VIDEO);
    m_window = SDL_CreateWindow(
            app.title(),
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            m_screen_width, m_screen_height,
            SDL_WINDOW_RESIZABLE);

    m_renderer = SDL_CreateRenderer(m_window, -1, SDL_RENDERER_PRESENTVSYNC);
    //SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_ADD);

    // font
    std::vector<uint16_t> data(16 * 6 * 8 * 8);
    for (int i = 0; i < (int) data.size(); ++i) {
        data[i] = (FONT[i / 8] & (1 << (7 - i % 8))) ? 0xffff : 0;
    }
    m_font_tex = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_STATIC, 16 * 8, 6 * 8);
    SDL_UpdateTexture(m_font_tex, nullptr, data.data(), 2 * 16 * 8);
    SDL_SetTextureBlendMode(m_font_tex, SDL_BLENDMODE_BLEND);


    app.init();

    m_running = true;
    while (m_running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_QUIT:
                m_running = false;
                break;

            case SDL_KEYDOWN:
                if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) m_running = false;
                else app.key(e.key.keysym.scancode);
                break;

            case SDL_TEXTINPUT:
                app.textinput(e.text.text);
                break;

            case SDL_WINDOWEVENT:
                if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    m_screen_width  = e.window.data1;
                    m_screen_height = e.window.data2;
                }
                break;


            default: break;
            }
        }

        app.update();
        SDL_RenderPresent(m_renderer);
    }

    SDL_DestroyTexture(m_font_tex);
    SDL_DestroyRenderer(m_renderer);
    SDL_DestroyWindow(m_window);
    SDL_Quit();
    return 0;
}

void clear() {
    SDL_RenderClear(m_renderer);
}

void set_color(int r, int g, int b, int a) {
    SDL_SetRenderDrawColor(m_renderer, r, g, b, a);
}

void draw_line(int x1, int y1, int x2, int y2) {
    SDL_RenderDrawLine(m_renderer, x1, y1, x2, y2);
}

void draw_rectangle(bool fill, int x, int y, int w, int h) {
    SDL_Rect r = { x, y, w, h };
    if (fill) SDL_RenderFillRect(m_renderer, &r);
    else      SDL_RenderDrawRect(m_renderer, &r);
}

void set_font_color(int r, int g, int b) {
    SDL_SetTextureColorMod(m_font_tex, r, g, b);
}

void put_char(int x, int y, char c) {
    if (c < 32) return;
    SDL_Rect src = { c % 16 * 8, (c - 32) / 16 * 8, 8, 8 };
    SDL_Rect dst = { x, y, 16, 16 };
    SDL_RenderCopy(m_renderer, m_font_tex, &src, &dst);
}

void print(int x, int y, const char* str) {
    while (*str) {
        put_char(x, y, *str);
        ++str;
        x += 16;
    }
}

void printf(int x, int y, const char* format, ...) {
    va_list args;
    va_start(args, format);
    char line[256];
    vsnprintf(line, 256, format, args);
    va_end(args);
    print(x, y, line);
}

int screen_width()  { return m_screen_width; }
int screen_height() { return m_screen_height; }

} // namespace
