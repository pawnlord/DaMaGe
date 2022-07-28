#ifndef GBPROCESSOR_H
#define GBPROCESSOR_H
#include "gbppu.h"
#include "gbmem.h"
#include <unordered_map>
#include <cstdint>
#include <cmath>
#include <queue>
#include <ctime>
#define DEFAULT_SPEED 4194

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

class Clock{ // Also manages delta time
    public:
    Clock(Memory *mem);
    void tick();
    void set_speed(int ops_per_mill);

    private:
    int ops_per_mill = DEFAULT_SPEED;
    int count;
    Memory *mem;
    timereg_t *timereg;
    int clock_freq;
    std::clock_t last_time;
    int dot_diff = 0;
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
    
    void print_info();
    void set_change_speed(double ops_mult);
    void load_savestate(std::string name);
    void save_savestate();
    private:
    gbreg* get_first_arg(int lowbyte, regtype_e* ishigh);
    gbreg* get_last_arg(int lowbyte, regtype_e* ishigh);
    
    void setbcdflags(uint8_t before, uint8_t operand, bool issub);
    void setbcddir(bool ishc, bool issub);
    void clearbcd();
    
    bool tick(uint8_t cpu_cycles);
    
    void halt();
    void push(uint16_t dat);
    void pop(uint16_t* dat);
    void cmp(int a, int b);
    
    bool getflag(int flag);
    void setflag(int flag, bool set);
    
    bool jmp(condition_e con, bool isCall, bool isNot);
    bool ret(condition_e con, bool isNot);
    void reti();
    
    void savestate_handler();
    
    int prefixop(uint8_t opcode);
    Memory *mem;
    Clock *clock;
    PPU *ppu;
    
    bool IME, halt_flag;
    regs_t regs;
    
    // points to AF.r8l
    uint8_t *flags;
    int change_speed = DEFAULT_SPEED * 2;
    
    // interrupt logic
    // pointers to interrupt flags
    uint8_t *IE, *IF;
    bool debug = false;
    uint32_t cycles_to_run = 1;    
};

#endif