#include "gbprocessor.h"

CPU::CPU(Memory *mem, Clock*clock){
    this->mem = mem;
    this->clock = clock;
    flags = &AF.hl.r8l;
    IME = true;
    halt_flag = false;
    IE = mem->getref(0xFFFF);
    IF = mem->getref(0xFF0F);
}

void CPU::populate_ops(){
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
}

void PPU::tick(){

}

Clock::Clock(Memory *mem){
    this->mem = mem;
    this->timereg = mem->timereg;
    count = 0;
}

// 4 Clock ticks at once
void Clock::tick(){
    count += 4;
    bool timer_enabled = timereg->TAC & 0x4;
    if(timer_enabled){
        int tac_type = timereg->TAC & 0x3;
        if(tac_type == 0){
            tac = 1024;
        } else {
            tac = 4*pow(4, tac_type);
        }
        if(count % tac == 0){
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