#include <SDL2/SDL.h>
#include "fx.hpp"

namespace fx {
namespace {

SDL_Window*   m_window;
SDL_Renderer* m_renderer;
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

//    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_ADD);

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


int screen_width()  { return m_screen_width; }
int screen_height() { return m_screen_height; }

} // namespace
