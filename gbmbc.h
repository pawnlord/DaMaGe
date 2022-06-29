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
    uint8_t get(uint16_t addr); //0x0-0x7FFF
    void set(uint16_t addr, uint8_t val);
    uint8_t* full;
private:
    void setbank2();
    char* externalmem;
    uint8_t lowbankreg = 0, hibankreg = 0;
    bool RAMEnabled = false;
    bool mode = false;
    int size;
};



#endif