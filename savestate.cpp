#include "savestate.h"
#define HEADER_SIZE 28
#define RAM_SIZE 0x10000

void savestate_t::add_rom(uint8_t *fullrom, int size){
    if(rom != nullptr){ free(rom); }
    rom = (uint8_t*)malloc(size);
    rom_size = size;
    std::memcpy(rom, fullrom, size);
}
void savestate_t::add_ext_ram(uint8_t *extram, int size){
    if(this->extram != nullptr){ free(this->extram); }
    this->extram = (uint8_t*)malloc(size);
    external_ram_size = size;
    std::memcpy(this->extram, extram, size);
}
void savestate_t::add_ram(uint8_t *ram){
    if(this->ram != nullptr){ free(this->ram); }
    this->ram = (uint8_t*)malloc(RAM_SIZE);
    std::memcpy(this->ram, ram, RAM_SIZE);
}

void add_number(uint8_t *buf, int num, int idx, int size){
    for(int i = 0; i < size; i++){
        buf[i+idx] = (num/(1<<(i*8)))%0x100;
    }
}

int read_number(uint8_t *buf, int idx, int size){
    int num = 0;
    for(int i = 0; i < size; i++){
        uint8_t dig = buf[i+idx];
        num += (dig*(1<<(i*8)));
    }
    return num;
}

void savestate_t::save(std::string title){
    std::ofstream ofs (title + ".sav", std::ofstream::binary);
    uint8_t * header = (uint8_t*)malloc(HEADER_SIZE); // size subject to change
    add_number(header, rom_size, 0, 4);
    add_number(header, external_ram_size, 4, 4);
    add_number(header, rom_size+HEADER_SIZE, 8, 4); // pointer to external ram
    add_number(header, rom_size+external_ram_size+HEADER_SIZE, 12, 4); // pointer to ram
    for(int i = 0; i < 6; i++){
        add_number(header, regs[i], 16+(i*2), 2); 
    }
    ofs.write((char*)header, HEADER_SIZE);
    ofs.write((char*)this->rom, this->rom_size);
    ofs.write((char*)this->extram, this->external_ram_size);
    ofs.write((char*)this->ram, RAM_SIZE);
    ofs.close();
}

void savestate_t::load(std::string title){
    std::ifstream ifs (title + ".sav", std::ifstream::binary);
    std::filebuf* pbuf = ifs.rdbuf();
    
    std::size_t size = pbuf->pubseekoff (0,ifs.end,ifs.in);
    uint8_t *raw_buf = (uint8_t*)malloc(size*sizeof(uint8_t));
    pbuf->pubseekpos (0,ifs.in);
    
    pbuf->sgetn((char*)raw_buf, size);    
    
    ifs.close();

    this->rom_size = read_number(raw_buf, 0, 4);
    this->external_ram_size = read_number(raw_buf, 4, 4);
    int extramp = read_number(raw_buf, 8, 4);
    int ramp = read_number(raw_buf, 12, 4);
    for(int i = 0; i < 6; i++){
        regs[i] = read_number(raw_buf, 16+(i*2), 2); 
    }

    add_rom(raw_buf+HEADER_SIZE, rom_size);
    add_ext_ram(raw_buf+HEADER_SIZE+rom_size, external_ram_size);
    add_ram(raw_buf+HEADER_SIZE+rom_size+external_ram_size);
    
    free(raw_buf);
}

void savestate_t::add_reg(uint16_t r16){
    regs[regnum] = r16;
    regnum++;
}

