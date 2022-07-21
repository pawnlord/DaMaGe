#ifndef GBMEM_H
#define GBMEM_H
#include <cstdint>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include "constants.h"
#include "gbmbc.h"
#include "config.h"

struct cart_info{
    uint32_t entry_point;
    uint8_t logo[0x30];
    uint8_t title[0x10];
    uint16_t new_license_code;
    uint8_t SGB_flag;
    uint8_t MBC_type;
    uint8_t ROM_size;
    uint8_t RAM_size;
    uint8_t dest_code;
    uint8_t old_license_code;
    uint8_t vers_no;
    uint8_t head_cs;
    uint8_t glob_cs;
};

struct timereg_t{
    uint8_t DIV;
    uint8_t TIMA;
    uint8_t TMA;
    uint8_t TAC;
};

void print_cart_info(cart_info* inf);

uint8_t getmbctype(uint8_t* data);

MBC *mbc_from_file(std::string filename);
struct key_bindings_t{
    int up;
    int down;
    int left;
    int right;
    int a;
    int b;
    int select;
    int start;
    int speed_change;
};

class Memory{
    public:
    Memory(bool *input, EmulatorConfig cfg);
    void dump();
    uint8_t get(uint16_t addr);
    uint8_t& operator[](int);
    void set(uint8_t v, uint16_t addr);
    cart_info* load_cartridge(std::string filename);
    timereg_t* timereg;
    // return pointer into memory, for certain structures.
    uint8_t *getref(uint16_t addr);
    void req_int(uint8_t flags);
    void unreq_int(uint8_t flags);
    bool get_int(uint8_t flags);
    uint8_t get_int_num();
    bool get_int_enabled(uint8_t flags);
    void reset_int(uint8_t flags);
    void reset_regs();
    void tick();
    MBC *mbc;
    uint8_t *raw_mem;
    bool *input; // input from sdl
    bool is_change_speed();
    key_bindings_t kbs;
    private:
    uint8_t DMA_counter = 0;
    uint16_t DMA_highnib = 0;
    bool inDMA = false;
    uint8_t handle_input();
    cart_info inf;
};


#endif