#include "gbmbc.h"


void MBC::fromfile(std::string filename){
    std::ifstream ifs (filename, std::ifstream::binary);
    std::filebuf* pbuf = ifs.rdbuf();
    std::size_t size = pbuf->pubseekoff (0,ifs.end,ifs.in);
    this->size = size;
    full = (uint8_t*)malloc(size*sizeof(uint8_t));

    pbuf->pubseekpos (0,ifs.in);
    std::cout << size << std::endl;
    pbuf->sgetn((char*)full, size);
    ifs.close();

    externalmem = (char*)malloc(2<<14);
}
uint8_t MBC::get(uint16_t addr){
    if(addr <= 0x3FFF){
        uint32_t realaddr = (mode)? addr | (lowbankreg<<19):addr;
        return full[realaddr];
    } else if (addr <= 0x7FFF){
        uint32_t realaddr = (addr-0x4000) | (lowbankreg<<14) | (hibankreg<<19);
        std::cout << realaddr << "\n";
        return full[realaddr];
    } else {
        uint32_t realaddr = addr | (hibankreg<<14);
        return externalmem[realaddr];
    }
} //0x0-0x7FFF
void MBC::set(uint16_t addr, uint8_t val){
    if(addr <= 0x1FFF){
        RAMEnabled = (val & 0xA) == 0xA;
    } else if(addr <= 0x3FFF){
        lowbankreg = val&(0b11111);
    } else if(addr <= 0x5FFF){
        hibankreg = val&0b11;
    } else if (addr <= 0x7FFF){
        mode = val > 0;
    } else if(addr <= 0x9FFF){
        return;
    } else {
        uint32_t realaddr = (addr-0xA000) | ((mode)?(hibankreg<<13):0);
        externalmem[realaddr] = val;
    }
}