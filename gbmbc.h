#ifndef GBMBC_H
#define GBMBC_H
#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include <stdlib.h>
#define EXTMEM_REGION 0xBFFF - 0xA000
// Memory Bank Controller for a cartridge

class MBC{
public:
    virtual void fromfile(std::string filename) = 0;
    virtual void fromraw(uint8_t *&&data) = 0;
    virtual uint8_t get(uint16_t addr) = 0;
    virtual uint8_t *getref(uint16_t addr) = 0;
    virtual void set(uint16_t addr, uint8_t val) = 0;
    virtual void tick() = 0;// unused by MBC 1
    virtual void set_size(int sz) = 0;
    virtual void print_info() = 0;
    virtual bool isramenabled() = 0;
    virtual int get_size();
    virtual int get_ext_ram_size() = 0;
    virtual uint8_t *get_ext_mem() = 0;
    virtual uint8_t *get_full() = 0;
};

class MBCNone : MBC{
    public: 
    virtual void fromfile(std::string filename);
    virtual void fromraw(uint8_t *&&data);
    virtual uint8_t get(uint16_t addr);
    virtual uint8_t *getref(uint16_t addr);
    virtual void set(uint16_t addr, uint8_t val);
    virtual void tick(); // unused by MBC 1
    virtual void set_size(int sz);
    virtual void print_info();
    virtual bool isramenabled();
    virtual int get_size();
    virtual int get_ext_ram_size();
    virtual uint8_t *get_ext_mem();
    virtual uint8_t *get_full();
    private:
    uint8_t *externalmem;
    uint8_t *full;
};

class MBC1 : MBC
{
public:
    virtual void fromfile(std::string filename);
    virtual void fromraw(uint8_t *&&data);
    virtual uint8_t get(uint16_t addr);
    virtual uint8_t *getref(uint16_t addr);
    virtual void set(uint16_t addr, uint8_t val);
    uint8_t* full;
    virtual void tick(){} // unused by MBC 1
    virtual void set_size(int sz){ this->size = sz;}
    virtual void print_info();
    virtual bool isramenabled();
    virtual int get_size();
    virtual int get_ext_ram_size();
    virtual uint8_t *get_ext_mem();
    virtual uint8_t *get_full() {return full;};
protected:
    uint8_t* externalmem;
    uint8_t lowbankreg = 0, hibankreg = 0;
    bool RAMEnabled = false;
    bool mode = false;
    int size;
};

class MBC3 : public MBC1{
    virtual void fromfile(std::string filename);
    virtual void fromraw(uint8_t *&&data);
    virtual uint8_t get(uint16_t addr); 
    virtual uint8_t *getref(uint16_t addr);
    virtual void set(uint16_t addr, uint8_t val);
    virtual void tick();
    virtual bool isramenabled();
    virtual int get_ext_ram_size();
    virtual uint8_t *get_full() {return full;};
    private:
    uint8_t last_latch = 0xff;
    bool latched = false;
    uint8_t regs[5];
    uint8_t ram_select = 0;
};



#endif