#ifndef GBMEM_H
#define GBMEM_H
#include <cstdint>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cstring>
#include "constants.h"

struct cart_info{
    int32_t entry_point;
    int8_t logo[0x30];
    int8_t title[0x10];
    int8_t CGB_flag;
    int16_t new_license_code;
    int8_t SGB_flag;
    int8_t MBC_type;
    int8_t ROM_size;
    int8_t RAM_size;
    int8_t dest_code;
    int8_t old_license_code;
    int8_t vers_no;
    int8_t head_cs;
    int8_t glob_cs;
};

struct timereg_t{
    uint8_t DIV;
    uint8_t TIMA;
    uint8_t TMA;
    uint8_t TAC;
};

void print_cart_info(cart_info* inf);

class Memory{
    public:
    Memory();
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
    void reset_int(uint8_t flags);
    void reset_regs();
    private:
    uint8_t *raw_mem;
    cart_info inf;
};

#endif