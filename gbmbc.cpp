#include "gbmbc.h"
#include <time.h>
#include <cstring>

void MBCNone::fromfile(std::string filename){
    std::ifstream ifs (filename, std::ifstream::binary);
    std::filebuf* pbuf = ifs.rdbuf();
    std::size_t size = pbuf->pubseekoff (0,ifs.end,ifs.in);
    full = (uint8_t*)malloc(0x7FFF);
    std::memset(full, 0, 0x7FFF);

    pbuf->pubseekpos (0,ifs.in);
    std::cout << size << std::endl;
    pbuf->sgetn((char*)full, size);
    ifs.close();

    externalmem = (uint8_t*)malloc(EXTMEM_REGION);
    memset(externalmem, 0, EXTMEM_REGION);
}
void MBCNone::fromraw(uint8_t *&&data){
    full = (uint8_t*)realloc(data, 0x7FFF);
    externalmem = (uint8_t*)malloc(EXTMEM_REGION);
    memset(externalmem, 0, EXTMEM_REGION);
}
uint8_t MBCNone::get(uint16_t addr){
    if(addr <= 0x7FFF){
        return full[addr];
    }
    return externalmem[addr-0xA000];
}
uint8_t *MBCNone::getref(uint16_t addr){
    if(addr <= 0x7FFF){
        return full+addr;
    }
    return externalmem+addr-0xA000;
}
void MBCNone::set(uint16_t addr, uint8_t val){
    if(addr <= 0x7FFF){
        full[addr] = val;
    }
    externalmem[addr-0xA000] = val;
}
void MBCNone::tick(){} // unused by no MBC
void MBCNone::set_size(int sz){}
void MBCNone::print_info(){
    std::cout << "No MBC\n";
}
bool MBCNone::isramenabled(){ return true; }
int MBCNone::get_size() {return 0x7FFF;}
int MBCNone::get_ext_ram_size(){return EXTMEM_REGION;}
uint8_t *MBCNone::get_ext_mem(){return externalmem;}
uint8_t *MBCNone::get_full() {return full;}





void MBC1::fromfile(std::string filename){
    std::ifstream ifs (filename, std::ifstream::binary);
    std::filebuf* pbuf = ifs.rdbuf();
    std::size_t size = pbuf->pubseekoff (0,ifs.end,ifs.in);
    this->size = size;
    full = (uint8_t*)malloc(size*sizeof(uint8_t));

    pbuf->pubseekpos (0,ifs.in);
    std::cout << size << std::endl;
    pbuf->sgetn((char*)full, size);
    ifs.close();

    externalmem = (uint8_t*)malloc(2<<14);
}
void MBC1::fromraw(uint8_t *&&data){
    full = data;
    externalmem = (uint8_t*)malloc(2<<14);
}

bool MBC1::isramenabled(){
    return RAMEnabled;
}
int MBC1::get_size(){ return size; }

int MBC1::get_ext_ram_size(){ return 2<<14; }

uint8_t MBC1::get(uint16_t addr){
    if(addr <= 0x3FFF){
        uint32_t realaddr = (mode)? addr | (hibankreg<<19):addr;
        return full[realaddr];
    } else if (addr <= 0x7FFF){
        uint32_t realaddr = (addr - 0x4000) | (lowbankreg<<14) | (hibankreg<<19);
        if(lowbankreg == 0 && hibankreg == 0){
            realaddr += 0x4000;
        }
        return full[realaddr];
    } else {
        uint32_t realaddr = (mode)? (addr - 0xA000) : (addr - 0xA000) | (hibankreg<<13);
        return externalmem[realaddr];
    }
}

uint8_t *MBC1::getref(uint16_t addr){
   if(addr <= 0x3FFF){
        uint32_t realaddr = (mode)? addr | (hibankreg<<19):addr;
        return full+realaddr;
    } else if (addr <= 0x7FFF){
        uint32_t realaddr = (addr - 0x4000) | (lowbankreg<<14) | (hibankreg<<19);
        if(lowbankreg == 0 && hibankreg == 0){
            realaddr += 0x4000;
        }
        return full+realaddr;
    } else {
        uint32_t realaddr = (mode)? (addr - 0xA000) : (addr - 0xA000) | (hibankreg<<13);
        return externalmem+realaddr;
    }
}
void MBC1::set(uint16_t addr, uint8_t val){
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
void MBC1::print_info(){
    std::cout << "MBC Bank: " << (lowbankreg + (hibankreg<<0x5)) << "(mode: " << mode << ")" << std::endl;
    std::cout << "RAM Enabled: " << RAMEnabled << std::endl;

}
uint8_t *MBC1::get_ext_mem(){ return externalmem; }



void MBC3::fromfile(std::string filename){
    std::ifstream ifs (filename, std::ifstream::binary);
    std::filebuf* pbuf = ifs.rdbuf();
    std::size_t size = pbuf->pubseekoff (0,ifs.end,ifs.in);
    this->size = size;
    full = (uint8_t*)malloc(size*sizeof(uint8_t));

    pbuf->pubseekpos (0,ifs.in);
    std::cout << size << std::endl;
    pbuf->sgetn((char*)full, size);
    ifs.close();

    externalmem = (uint8_t*)malloc(2<<14);
}
void MBC3::fromraw(uint8_t *&&data){
    full = data;
    externalmem = (uint8_t*)malloc(2<<14);
}
bool MBC3::isramenabled(){
    return RAMEnabled;
}

int MBC3::get_ext_ram_size(){ return 2<<14; }

uint8_t MBC3::get(uint16_t addr){
    if(addr <= 0x3FFF){
        uint32_t realaddr = addr;
        return full[realaddr];
    } else if (addr <= 0x7FFF){
        uint32_t realaddr = (addr-0x4000) | (lowbankreg<<14) | (hibankreg<<19);
        if(lowbankreg == 0 && hibankreg == 0){
            realaddr += 0x4000;
        }
        return full[realaddr];
    } else {
        if(ram_select <= 3){
            uint32_t realaddr = (addr-0xA000) | (ram_select<<13);
            return externalmem[realaddr];
        } else {
            return regs[ram_select-8];
        }
    }
}


uint8_t *MBC3::getref(uint16_t addr){
   if(addr <= 0x3FFF){
        uint32_t realaddr = (mode)? addr | (hibankreg<<19):addr;
        return full+realaddr;
    } else if (addr <= 0x7FFF){
        uint32_t realaddr = (addr - 0x4000) | (lowbankreg<<14) | (hibankreg<<19);
        if(lowbankreg == 0 && hibankreg == 0){
            realaddr += 0x4000;
        }
        return full+realaddr;
    } else {
        uint32_t realaddr = (mode)? (addr - 0xA000) : (addr - 0xA000) | (hibankreg<<13);
        return externalmem+realaddr;
    }
}
void MBC3::set(uint16_t addr, uint8_t val){

    if(addr <= 0x1FFF){
        RAMEnabled = (val & 0xA) == 0xA; // Also enables timer
    } else if(addr <= 0x3FFF){
        lowbankreg = val&(0b11111);
        hibankreg = (val&0b1100000)>>5;
    } else if(addr <= 0x5FFF){
        ram_select = (val&0b1111);
    } else if (addr <= 0x7FFF){
        if(val == 0x1 && last_latch == 0){
            latched = !latched;
        }
        last_latch = val;
    } else if(addr <= 0x9FFF){
        return;
    } else {
        if(ram_select <= 3){
            uint32_t realaddr = (addr-0xA000) | (ram_select<<13);
            externalmem[realaddr] = val;
        } else {
            regs[ram_select-8] = val; 
        }
    }
}

void MBC3::tick(){
    // stores time
    if(!latched){
        if(regs[4] & (1<<6)){
            time_t curr = time(NULL);
            tm *t = localtime(&curr);
            regs[0] = t->tm_sec;
            regs[1] = t->tm_min;
            regs[2] = t->tm_hour;
            regs[3] = t->tm_yday & 0xFF;
            regs[4] = (regs[4] & ~0x1) | ((t->tm_yday & 0x100)>>8);
            regs[4] = (regs[4] & ~0x40);
            
        }
    }
}
