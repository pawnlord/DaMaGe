#ifndef GBPROCESSOR_H
#define GBPROCESSOR_H
#include <cstdint>
#include "gbmem.h"
#include <unordered_map>
#include <cmath>
#include <queue>
#include "gbdisplay.h"


union gbreg {
    struct {
        uint8_t r8l;        
        uint8_t r8h;
    } hl;
    uint16_t r16;
    gbreg& operator=(uint16_t);
};

struct regs_t{
    gbreg AF, BC, DE, HL, SP, PC;    
};

class Clock{
    public:
    Clock(Memory *mem);
    void tick();

    private:
    int count;
    Memory *mem;
    timereg_t *timereg;
    int tac;
};

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
    bool getlcdc(int value);

    Memory *mem;
    layer bg, window, objects;
    
    uint16_t *tile_ref;
    uint8_t *LCDC;
    uint8_t *LCDStat;
   
    object_t *OAM;

    enum mode_e {M0 = 0, M1, M2, M3} mode;
    std::queue<pixel_t> fifo;

};

enum regtype_e {
    HIGH, LOW, FULL
};

class CPU{
    public:
    CPU(Memory *mem, Clock *clock);
    void run();
    private:
    gbreg* get_first_arg(int lowbyte, regtype_e* ishigh);
    gbreg* get_last_arg(int lowbyte, regtype_e* ishigh);
    bool tick(uint8_t cpu_cycles);
    void halt();
    void push(uint16_t dat);
    void pop(uint16_t* dat);
    void cmp(int a, int b);
    Memory *mem;
    Clock *clock;
    PPU *ppu;
    bool IME, halt_flag;
    regs_t regs;
    // points to AF.r8l
    uint8_t *flags;

    // interrupt logic
    // pointers to interrupt flags
    uint8_t *IE, *IF;    
};

#endif