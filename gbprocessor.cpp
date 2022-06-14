#include "gbprocessor.h"

gbreg& gbreg::operator=(const uint16_t a){
    this->r16 = a;
    return *this;
}

CPU::CPU(Memory *mem, Clock*clock){
    this->mem = mem;
    this->clock = clock;
    PC = 0x100;
    AF.hl.r8h = 0x1;
    BC = 0x13;
    DE = 0xC1;
    HL = 0x8403;
    SP = 0xFFFE;
    flags = &AF.hl.r8l;
    (*flags) = 0b1011;
    IME = true;
    halt_flag = false;
    IE = mem->getref(0xFFFF);
    IF = mem->getref(0xFF0F);
}

void nop_f(uint32_t i){}

void CPU::populate_ops(){
    op_t nop(1, nop_f);
    op_t (1, nop_f);
    ops[0x0] = nop;
}



void CPU::tick(uint8_t cpu_cycles){
    for(int i = 0; i < cpu_cycles; i++){
        clock->tick();
        ppu->tick();
    }
}

PPU::PPU(Memory *mem){
    this->mem = mem;
    this->tile_ref = (uint16_t*)mem->getref(0x8000);
    this->LCDC = mem->getref(0xFF40);
    this->LCDStat = mem->getref(0xFF41);
    this->OAM = (object_t*)mem->getref(0xFE00);
    this->mode = M2; 
}

bool PPU::getlcdc(int val){
    int temp = (int)(pow(2, val));
    if (temp & (*LCDC) > 0){
        return true;
    }
    return false;
}

void PPU::tick(){
    if(getlcdc(7)){

        if(fifo.empty()){

        }
    }
}

Clock::Clock(Memory *mem){
    this->mem = mem;
    this->timereg = mem->timereg;
    count = 0;
}

// 4 Clock ticks at once
void Clock::tick(){
    count += 1;
    bool timer_enabled = timereg->TAC & 0x4;
    if(timer_enabled){
        int tac_type = timereg->TAC & 0x3;
        if(tac_type == 0){
            tac = 1024;
        } else {
            tac = 4*pow(4, tac_type);
        }
        if(count % tac == 0 ){
            timereg->TIMA+=1;
            if(timereg->TIMA == 0){
                mem->req_int(0x4); // Request Timer interrupt
                timereg->TIMA = timereg->TMA;
            }
        }
        if(count % 256 == 0){
            timereg->DIV += 1;
        }
    }
    count %= 4194304; // cycles per second 
    // TODO: Possibly delay here.
}