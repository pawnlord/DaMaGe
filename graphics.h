#ifndef GRAPHICS_H
#define GRAPHICS_H
#define SDL_MAIN_HANDLED 
#ifdef __MINGW32__
    #define _WIN32_WINNT 0x0501
    #include <mingw.thread.h>
    #include <mingw.mutex.h>
#else
    #include <thread>
    #include <mutex>
#endif
#include <SDL2/SDL.h>
#include <cstdlib>
#include <iostream>
#include <vector>
struct Pixel : SDL_Rect{
    int r,g,b;
    Pixel(int x, int y, int s, int r, int g, int b);
    void set_color(int r, int g, int b);
    void set_color(int v);
};

class GraphicsManager{
    public:
    void sdl_check_loud(void* value);
    void sdl_check_loud(void* value, void* against);


    GraphicsManager(std::string name, int w, int h, int s, int head);
    // starts render thread
    void start();
    // render thread callback
    void render_cb();

    void add_texture();

    void stopSDL();

    private:
    int w, h;
    bool running = true;
    std::mutex runmx;
    SDL_Window* win;
    SDL_Renderer* ren;
    std::thread graphicsThread;
    std::vector<std::vector<Pixel> > pixs; 
};
#endif