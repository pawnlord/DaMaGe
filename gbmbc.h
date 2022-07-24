#ifndef GBMBC_H
#define GBMBC_H
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include <stdlib.h>
// Memory Bank Controller for a cartridge

class MBC
{
public:
    virtual void fromfile(std::string filename);
    virtual void fromraw(uint8_t* data);
    virtual uint8_t get(uint16_t addr);
    virtual uint8_t *getref(uint16_t addr);
    virtual void set(uint16_t addr, uint8_t val);
    uint8_t* full;
    virtual void tick(){} // unused by MBC 1
    virtual void set_size(int sz){ this->size = sz;}
    virtual void print_info();
    virtual bool isramenabled();
protected:
    uint8_t* externalmem;
    uint8_t lowbankreg = 0, hibankreg = 0;
    bool RAMEnabled = false;
    bool mode = false;
    int size;
};

class MBC3 : public MBC{
    virtual void fromfile(std::string filename);
    virtual void fromraw(uint8_t* data);
    virtual uint8_t get(uint16_t addr); 
    virtual uint8_t *getref(uint16_t addr);
    virtual void set(uint16_t addr, uint8_t val);
    virtual void tick();
    virtual bool isramenabled();
    private:
    uint8_t last_latch = 0xff;
    bool latched = false;
    uint8_t regs[5];
    uint8_t ram_select = 0;
};



#endif