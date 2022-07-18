#ifndef GBMBC_H
#define GBMBC_H
#include <cstdint>
#include <fstream>
#include <iostream>
#include <stdlib.h>
// Memory Bank Controller for a cartridge

class MBC
{
public:
    void fromfile(std::string filename);
    void fromraw(uint8_t* data);
    uint8_t get(uint16_t addr);
    void set(uint16_t addr, uint8_t val);
    uint8_t* full;
    void tick(){} // unused by MBC 1
    void set_size(int sz){ this->size = sz;}
protected:
    char* externalmem;
    uint8_t lowbankreg = 0, hibankreg = 0;
    bool RAMEnabled = false;
    bool mode = false;
    int size;
};

class MBC3 : public MBC{
    void fromfile(std::string filename);
    void fromraw(uint8_t* data);
    uint8_t get(uint16_t addr); 
    void set(uint16_t addr, uint8_t val);
    void tick();
    private:
    uint8_t regs[5];
    uint8_t ram_select = 0;
};



#endif