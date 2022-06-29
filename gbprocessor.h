#ifndef GBPROCESSOR_H
#define GBPROCESSOR_H
#include "gbppu.h"
#include "gbmem.h"
#include <unordered_map>
#include <cstdint>
#include <cmath>
#include <queue>


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


enum regtype_e {
    HIGH, LOW, FULL
};
enum condition_e{
    CON_NONE, CON_ZERO, CON_CARRY
};

class CPU{
    public:
    CPU(Memory *mem, Clock *clock);
    void run();
    PPU *getppu();
    private:
    gbreg* get_first_arg(int lowbyte, regtype_e* ishigh);
    gbreg* get_last_arg(int lowbyte, regtype_e* ishigh);
    void setbcdflags(uint8_t before, uint8_t after, bool issub);
    void setbcddir(bool ishc, bool issub);
    void clearbcd();
    bool tick(uint8_t cpu_cycles);
    void halt();
    void push(uint16_t dat);
    void pop(uint16_t* dat);
    void cmp(int a, int b);
    bool getflag(int flag);
    bool jmp(condition_e con, bool isCall, bool isNot);
    bool ret(condition_e con, bool isNot);
    void reti();
    int prefixop(uint8_t opcode);
    Memory *mem;
    Clock *clock;
    PPU *ppu;
    bool IME, halt_flag;
    regs_t regs;
    // points to AF.r8l
    uint8_t *flags;
    uint8_t *tempp;

    // interrupt logic
    // pointers to interrupt flags
    uint8_t *IE, *IF;
    bool debug = true;
    uint32_t cycles_to_run = 1;    
};

#endif