#ifndef GBPPU_H
#define GBPPU_H
#include <cstdint>
#include <cmath>
#include <queue>
#include "gbmem.h"
#include "gbdisplay.h"


struct object_t{
    uint8_t y;
    uint8_t x;
    uint8_t idx;
    uint8_t flag;
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
    private:
    void updt_oamscan();
    void updt_drawpxl();
    bool setlcdc(int value);
    bool getlcdc(int value);

    Memory *mem;
    layer bg, window, objects;
    
    uint16_t *tile_ref;
    uint8_t *LCDC;
    uint8_t *STAT;
   
    object_t *OAM;

    enum mode_e {M0 = 0, M1, M2, M3} mode;
    std::queue<pixel_t> fifo;
    int dots = 0;
    uint8_t *LY, *LYC;
};


#endif