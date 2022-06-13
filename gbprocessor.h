#ifndef GBPROCESSOR_H
#define GBPROCESSOR_H
#include <cstdint>
#include "gbmem.h"
#include <unordered_map>
#include <cmath>
#include "gbdisplay.h"

typedef union {
    struct {
        uint8_t r8l;        
        uint8_t r8h;
    } hl;
    uint16_t r16;
} gbreg;

typedef void (*opfunc_t)(uint32_t);

void nop_f(uint32_t i){}

struct op_t{
    // CPU Cycles, clock cycles/4
    int cycles;
    opfunc_t opfunc;
    op_t(int cycles, opfunc_t opfunc){
        this->cycles = cycles;
        this->opfunc = opfunc;
    }
} nop(1, nop_f);

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

class PPU{
    public:
    PPU(Memory *mem);
    void tick();
    private:
    Memory *mem;
    layer bg, window, objects;
};

class CPU{
    public:
    CPU(Memory *mem, Clock *clock);
    private:
    void populate_ops();
    void tick(uint8_t cpu_cycles);
    Memory *mem;
    Clock *clock;
    PPU *ppu;
    std::unordered_map<uint8_t, op_t> ops;    
    bool IME, halt_flag;
    gbreg AF, BC, DE, HL, SP, PC;
    // points to AF.r8l
    uint8_t *flags;

    // interrupt logic
    // pointers to interrupt flags
    uint8_t *IE, *IF;    
};

#endif