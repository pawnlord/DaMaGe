#ifndef GBPPU_H
#define GBPPU_H
#include <cstdint>
#undef _WIN32_WINNT
#include <cmath>
#include <vector>
#include <queue>
#include "gbmem.h"
#include <cmath>
#include <algorithm>

#define WIDTH 160
#define HEIGHT 144

typedef uint8_t **layer;


struct object_t{
    uint8_t y;
    uint8_t x;
    uint8_t idx;
    uint8_t flag;
    bool operator==(const object_t& rhs);
};


struct pixel_t{
    uint8_t color;
    uint8_t palette;
    uint8_t bgpriority;
};

class PPU{
    public:
    PPU(Memory *mem);
    void tick();
    //layer bg, window, objects;
    uint8_t getstatfull();
    uint8_t getlcdcfull();
    bool ppu_enabled();
    layer lcd;
    private:
    void init_drawpxl();
    void updt_oamscan();
    void updt_drawpxl();
    void pxl_fetcher();
    bool setlcdc(int value);
    bool getlcdc(int value);
    // helper functions
    uint8_t get_color(uint16_t fulltile, uint8_t i);

    Memory *mem;
    uint16_t *tile_ref;
    uint8_t *LCDC;
    uint8_t *STAT;
    bool is_paused = false, is_render_ready = false;
    uint8_t lineobjs = 0;
    uint8_t totalobjs = 0;
   
    object_t *OAM;
    std::vector<object_t> fetchedobjs;
    std::vector<object_t> currobjs;
    std::vector<pixel_t> fgfifo;
    int currobjidx;
    std::queue<pixel_t> bgfifo;
    enum mode_e {M0 = 0, M1, M2, M3} mode;
    int dots = 0;
    uint8_t *LY, *LYC, *SCX, *SCY, *WX, *WY;
    int fetchX = 0, displayX = 0, fetchWidth = WIDTH;
};


#endif