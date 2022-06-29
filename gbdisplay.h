#ifndef GBDISPLAY_H
#define GBDISPLAY_H
#define PIXEL_SIZE 3
#define HEADER 20
#include "gbppu.h"
#include "graphics.h"

// Layer between gameboy and SDL
class GameboyDisplay : MainLoop{
    public:
    GameboyDisplay(PPU* ppu);
    void update_gm_pixels();
    GraphicsManager gm;
    private:
    PPU* ppu;
    layer last_lcd;
};

#endif