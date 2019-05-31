#pragma once

namespace fx {
    struct App {
        virtual const char* title() const { return "app"; }
        virtual void init() {}
        virtual void key(int code) {}
        virtual void textinput(char const* text) {}
        virtual void update() {}
    };


    int run(App& App);
    void clear();
    void set_color(int r, int g, int b, int a = 255);
    void draw_line(int x1, int y1, int x2, int y2);
    void draw_rectangle(bool fill, int x, int y, int w, int h);
    int screen_width();
    int screen_height();
}
