#ifndef SAVESTATE_H
#define SAVESTATE_H
#include <fstream>
#include <stdlib.h>
#include <cstring>

void add_number(uint8_t *buf, int num, int idx, int size);
int read_number(uint8_t *buf, int idx, int size);

struct savestate_t{ // stores copies of rom, external ram, and memory
    int rom_size, external_ram_size;
    uint16_t regs[6];
    int regnum = 0;
    void add_rom(uint8_t *fullrom, int size);
    void add_ext_ram(uint8_t *extram, int size);
    void add_ram(uint8_t *ram);
    void add_reg(uint16_t r16);
    void get_reg(uint16_t r16);
    void load(std::string title);
    void save(std::string title);
    uint8_t *rom = nullptr, *ram = nullptr, *extram = nullptr;
};

#endif

