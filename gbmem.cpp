#include "gbmem.h"
#include <SDL2/SDL.h>

MBC *mbc_from_file(std::string filename){
    std::ifstream ifs (filename, std::ifstream::binary);
    std::filebuf* pbuf = ifs.rdbuf();
    std::size_t size = pbuf->pubseekoff (0,ifs.end,ifs.in);
    uint8_t *full = (uint8_t*)malloc(size*sizeof(uint8_t));

    pbuf->pubseekpos (0,ifs.in);
    pbuf->sgetn((char*)full, size);
    ifs.close();
    
    uint8_t t = getmbctype(full);
    MBC* mbc;
    switch (t){
        case 0:
        case 1:
        mbc = new MBC();
        mbc->fromraw(full);
        break;
        case 5:
        mbc = (MBC*)(new MBC3());
        mbc->fromraw(full);
        break;
        default:
        std::cout << "Unknown/WIP MBC\n";
        mbc = new MBC();
        mbc->fromraw(full);
        break;
    }
    mbc->set_size(size);
    return mbc;
}

uint8_t getmbctype(uint8_t* data){
    uint8_t raw_type = data[0x147];
    if(raw_type == 0){
        return 0;
    } else if(raw_type <= 0x3){
        return 1;
    } else if(raw_type <= 0x6){
        return 2;
    } else if(raw_type <= 0x9){
        return 3;
    } else if(raw_type <= 0xD){
        return 4;
    } else if(raw_type <= 0x13){
        return 5;
    } else if(raw_type <= 0x1E){
        return 6;
    } else if(raw_type == 0x20){
        return 7;
    } else if(raw_type == 0x22){
        return 8;
    } else {
        return -1;
    }
}

void print_cart_info(cart_info* inf) {
    std::cout << "Title: " << std::string((char*) inf->title) << std::endl;
}

Memory::Memory(bool* input, EmulatorConfig cfg){
    raw_mem = (uint8_t*) malloc(sizeof(uint8_t) * 0x10000);
    memset(raw_mem, 0, 0x10000);
    timereg = (timereg_t*)(raw_mem+0xFF04); // registers for dividers and timers 
    this->input = input;
    kbs.a = cfg.layout[0].key;
    kbs.b = cfg.layout[1].key;
    kbs.select = cfg.layout[2].key;
    kbs.start = cfg.layout[3].key;
    kbs.up = cfg.layout[4].key;
    kbs.down = cfg.layout[5].key;
    kbs.left = cfg.layout[6].key;
    kbs.right = cfg.layout[7].key;
    kbs.speed_change = cfg.layout[8].key;
}


void Memory::reset_regs(){
    set(0xCF, kP1);
    set(0x00, kSB);
    set(0x7E, kSC);
    set(0xAB, kDIV);
    set(0xAB, kDIV);
    set(0, kTIMA);
    set(0, kTMA);
    set(0xF8, kTAC);
    set(0x0, kIF);
    set(0x80, kNR1(0));
    set(0x80, kNR1(0));
    set(0x80, kNR1(0));
    set(0x80, kNR1(0));
    set(0x80, kNR1(0));
    set(0x80, kNR2(0));
    set(0x80, kNR2(0));
    set(0x80, kNR2(0));
    set(0x80, kNR2(0));
    set(0x80, kNR3(0));
    set(0x80, kNR3(0));
    set(0x80, kNR3(0));
    set(0x80, kNR3(0));
    set(0x80, kNR3(0));
    set(0x80, kNR4(0));
    set(0x80, kNR4(0));
    set(0x80, kNR4(0));
    set(0x80, kNR4(0));
    set(0x80, kNR5(0));
    set(0x80, kNR5(0));
    set(0x80, kNR5(0)); // TODO
    set(0x91, kLCDC);
    set(0x85, kSTAT);
    set(0, kSCY);
    set(0, kSCX);
    set(0, kLY);
    set(0, kLYC);
    raw_mem[kDMA] = 0xFF;
    set(0xFC, kBGP);
    set(0, kOBP0);
    set(0, kOBP1);
    set(0, kWY);
    set(0, kWX);
    set(0xFF, kKEY1);
    set(0, kIE);
}

uint8_t Memory::get(uint16_t addr){
    if(inDMA && addr < 0xFF80){
        return 0xFF; // Probably not the right value, but shouldn't be touched anyway
    }
    if(addr <= 0x7FFF || (addr >= 0xA000 && addr <= 0xBFFF && mbc->isramenabled())){
        return mbc->get(addr);
    }
    switch(addr){
        case 0xFF00:
            return handle_input();
        break;
        default:
            return raw_mem[addr];
        break;
    }
}

uint8_t Memory::handle_input(){
    uint8_t* inp_reg = raw_mem+0xFF00;
    uint8_t original = *inp_reg & 0xf; // for interrupt
    *inp_reg |= (0xCf); // reset to default state
    if((*inp_reg & (1<<4)) == 0){
        if (input[kbs.up]){
            *inp_reg &= ~(1<<2);
        }
        if(input[kbs.down]){
            *inp_reg &= ~(1<<3);
        }
        if (input[kbs.left]){
            *inp_reg &= ~(1<<1);
        }
        if (input[kbs.right]){
            *inp_reg &= ~(1<<0);
        }
    }
    if((*inp_reg & (1<<5)) == 0){
        if (input[kbs.a]){
            *inp_reg &= ~(1<<0);
        }
        if (input[kbs.b]){
            *inp_reg &= ~(1<<1);
        }
        if (input[kbs.select]){
            *inp_reg &= ~(1<<2);
        }
        if(input[kbs.start]){
            *inp_reg &= ~(1<<3);
        }
    }
    if((*inp_reg & 0xf) != original){
        req_int(1<<4);
    }
    return *inp_reg;
}
uint8_t& Memory::operator[](int idx){
    return raw_mem[idx];
}

bool Memory::is_change_speed(){
    return input[kbs.speed_change];
}


void Memory::print_changes(){
    for(int i = 0; i < changes.size(); i++){
        std::cout << std::dec << i << std::hex << ": 0x" << changes[i].addr << ", 0x" << (int)changes[i].oldb << "->0x" << (int)changes[i].newb << std::endl; 
    }
    changes.clear();
}

void Memory::set(uint8_t v, uint16_t addr){
    if(addr == 0x9100){
        std::cout << "backwards" << std::endl;
    }
    if(!inDMA || addr >= 0xFF80){
        if(addr <= 0x7FFF || (addr >= 0xA000 && addr <= 0xBFFF && mbc->isramenabled())){
            mbc->set(addr, v);
        } else{
            switch(addr){
                case 0xFF00:
                    raw_mem[addr] = (raw_mem[addr] & ~(0b11<<4)) | (v & ~0xCF);
                break;
                case 0xFF04:
                    timereg->DIV = 0;
                break;
                case 0xFF46:
                    DMA_highnib = v*0x100;
                    DMA_counter = 0;
                    inDMA = true;
                break;
                case 0xFF44:
                break;
                default:
                    raw_mem[addr] = v;
                break;
            }
        }
    }
}

void Memory::tick(){
    if(inDMA){
        raw_mem[0xFE00 + DMA_counter] = raw_mem[DMA_highnib + DMA_counter];
        DMA_counter+=1;
        if(DMA_counter == 0xA0){
            inDMA = false;
        }
    }
    mbc->tick();
}

void Memory::dump(){
    std::ofstream oftest ("test.dmp", std::ofstream::binary);
    oftest.write((char*)raw_mem, 0x10000);
    oftest.close();
}

cart_info* Memory::load_cartridge(std::string filename){
    mbc = mbc_from_file(filename);   
    // load cartridge into inf
    std::memcpy(&inf, mbc->full+0x100, sizeof(cart_info));
    reset_regs();
    print_cart_info(&inf);
    return &inf;
}


uint8_t* Memory::getref(uint16_t addr){
    return raw_mem+addr;
}

void Memory::req_int(uint8_t flags){
    raw_mem[0xFF0F] |= flags;
}
void Memory::unreq_int(uint8_t flags){
    raw_mem[0xFF0F] &= ~(flags);
}

bool Memory::get_int(uint8_t flags){
    return (raw_mem[0xFF0F] & flags) > 0;
}
bool Memory::get_int_enabled(uint8_t flags){
    return (raw_mem[0xFFFF] & flags) > 0;
}
void Memory::reset_int(uint8_t flags){
    raw_mem[0xFF0F] &= ~(flags);
}
uint8_t Memory::get_int_num(){
    for(int i = 0; i < 5; i++){
        if((raw_mem[0xFF0F] & ((uint8_t)(1<<i))) > 0){
            return i;
        }
    }
    return -1;
}
