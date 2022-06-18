#include "gbprocessor.h"

gbreg& gbreg::operator=(const uint16_t a){
    this->r16 = a;
    return *this;
}

CPU::CPU(Memory *mem, Clock*clock){
    this->mem = mem;
    this->clock = clock;
    regs.PC = 0x100;
    regs.AF.hl.r8h = 0x1;
    regs.BC = 0x13;
    regs.DE = 0xC1;
    regs.HL = 0x8403;
    regs.SP = 0xFFFE;
    flags = &regs.AF.hl.r8l;
    (*flags) = 0b1011;
    IME = true;
    halt_flag = false;
    IE = mem->getref(0xFFFF);
    IF = mem->getref(0xFF0F);
}

void CPU::halt(){
    while(true){
        if(tick(1)){
            return;
        }
    }
}


gbreg* CPU::get_last_arg(int lowbyte, regtype_e* ishigh){
    switch(lowbyte) {
        case 0x0:
            (*ishigh) = HIGH;
            return &regs.BC;
        break;
        case 0x1:
            (*ishigh) = LOW;
            return &regs.BC;
        break;
        case 0x2:
            (*ishigh) = HIGH;
            return &regs.DE;
        break;
        case 0x3:
            (*ishigh) = LOW;
            return &regs.DE;
        break;
        case 0x4:
            (*ishigh) = HIGH;
            return &regs.HL;
        break;
        case 0x5:
            (*ishigh) = LOW;
            return &regs.HL;
        break;
        case 0x6:
            (*ishigh) = FULL;
            return &regs.HL;
        break;
        case 0x7:
            (*ishigh) = HIGH;
            return &regs.AF;
        break;
    }
    return nullptr;
}

gbreg* CPU::get_first_arg(int fullbyte, regtype_e* ishigh){
    uint8_t h = (fullbyte/0x10)%4, l = fullbyte%0x10;
    (*ishigh) = (l >= 0x8)? LOW : HIGH; 
    switch(h){
        case 0x0:
            return &regs.BC;
        break;
        case 0x1:
            return &regs.DE;
        break;
        case 0x2:
            return &regs.HL;
        break;
        case 0x3:
            if((*ishigh) == HIGH){
                (*ishigh) = FULL;
                return &regs.HL;
            } else {
                (*ishigh) = HIGH;
                return &regs.AF;
            }
        break;
    }
    return nullptr;
}

int getval(gbreg* r, regtype_e t){
    if(t == HIGH){
        return r->hl.r8h;
    } else if(t == LOW){
        return r->hl.r8l;    
    } else {
        return r->r16;
    }
}

void setval(gbreg* r, regtype_e t, int val){
    if(t == HIGH){
        r->hl.r8h = (uint8_t)val;
    } else if(t == LOW){
        r->hl.r8l = (uint8_t)val;    
    } else {
        r->r16 = (uint8_t)val;
    }
}

void CPU::cmp(int a, int b){
    int cmp = a - b;
    if(cmp > 0){
        (*flags) &= ~(0x1<<0x7); // ZERO flag 
        (*flags) &= ~(0x1<<0x4); // CARRY flag 
    } else if(cmp == 0){
        (*flags) |= (0x1<<0x7); // ZERO flag
        (*flags) &= ~(0x1<<0x4); // CARRY flag 
    } else {
        (*flags) &= ~(0x1<<0x7); // ZERO flag 
        (*flags) |= (0x1<<0x4); // CARRY flag
    }

}

void CPU::run(){
    while(true){
        uint8_t opcode = mem->get(regs.PC.r16);
        std::cout << std::hex << "addr: " << regs.PC.r16 <<  ", opcode: "  << (int)opcode << std::endl;
        uint8_t h = opcode/0x10, l = opcode%0x10; // get highest bit
        int ticks = 1;
        // misc.
        if(opcode == 0x0){
            regs.PC.r16 += 1;
        }
        else if(h <= 0x3){
            regtype_e ishigh;
            gbreg* firstarg = get_first_arg(opcode, &ishigh);
            if(l == 0x0){
                uint16_t start = regs.PC.r16;
                uint16_t addr = start + (mem->get(++regs.PC.r16));
                ticks = 2;
                if((h == 0x2 && !(((*flags) & (0x1<<0x7)) > 0)) ||
                    (h == 0x3 && !(((*flags) & (0x1<<0x4)) > 0))){ 
                    regs.PC.r16 = addr;
                    ticks = 3;
                }
            }
            if(l == 0x6 ||  l == 0xE){
                regs.PC.r16 += 1;
                uint8_t val = mem->get(regs.PC.r16);
                setval(firstarg, ishigh, val);
                regs.PC.r16 += 1;
                ticks = 2;
                if(ishigh == FULL){
                    ticks = 3;
                }
            }
            if(l == 0x3){
                if(ishigh == FULL){
                    firstarg = &regs.SP;
                }
                setval(firstarg, FULL, getval(firstarg, FULL)+1);    
                ticks = 1;
            }
            if(l == 0xF){
                if(h == 0x0){
                    uint8_t lastbit = (regs.AF.hl.r8h & 0x1) << 7; // get last bit to keep
                    regs.AF.hl.r8h >>= 1;
                    regs.AF.hl.r8h == (regs.AF.hl.r8h & ~0x80) | lastbit; 
                } else if(h == 0x1){
                    uint8_t lastbit = (regs.AF.hl.r8h & 0x1) << 3; // get last bit to keep
                    uint8_t carrybit = ((*flags) & 0x8) << 4; 
                    (*flags) = ((*flags) & ~0x10) | lastbit;
                    regs.AF.hl.r8h >>= 1;
                    regs.AF.hl.r8h == (regs.AF.hl.r8h & ~0x80) | carrybit;     
                }
                ticks = 1;
                regs.PC.r16 += 1;
            }
        } 
        
        // Load
        else if(h >= 0x4 && h <= 0x7){
            regs.PC.r16 += 1;
            if(opcode == 0x77){
                halt();
            } else {
                if(l == 0x6 || l == 0xE){
                    ticks = 2;
                } else{
                    ticks = 1;
                }
            }
        }
        // operators
        else if(h >= 0x8 && h <= 0xB){
            regtype_e ishigh;
            gbreg* arg = get_last_arg(l%0x8, &ishigh);
            regs.PC.r16 += 1;
            if(l == 0x6 || l == 0xE){
                ticks = 2;
            } else{
                ticks = 1;
            }

            uint8_t* A = &regs.AF.hl.r8h;
            int val = (int)getval(arg, ishigh);
            if(h == 0x8){
                uint8_t carry = ((*A) + val < 0) << 0x4;
                uint8_t zero = ((*A) + val == 0) << 0x7;
                (*flags) = ((*flags) & (~0x10) & (~0x80)) | carry | zero;
                (*A) = (*A) + val;
            } else if(h == 0x9){
                uint8_t carry = ((*A) - val < 0) << 0x4;
                uint8_t zero = ((*A) - val == 0) << 0x7;
                (*flags) = ((*flags) & (~0x10) & (~0x80)) | carry | zero;
                (*A) = (*A) - val;
            } else if(h == 0xA){
                
            } else if(h == 0xB){
                
            }
        }
        // misc
        else if(h >= 0xC){
            // push/pop
            if(l == 0x5){
                switch(h){
                    case 0xC:
                        push(regs.BC.r16);
                    break;
                    case 0xD:
                        push(regs.DE.r16);
                    break;
                    case 0xE:
                        push(regs.HL.r16);
                    break;
                    case 0xF:
                        push(regs.AF.r16);
                    break;
                }
                ticks = 4;
            }
            else if(l == 0x1){
                switch(h){
                    case 0xC:
                        pop(&regs.BC.r16);
                    break;
                    case 0xD:
                        pop(&regs.DE.r16);
                    break;
                    case 0xE:
                        pop(&regs.HL.r16);
                    break;
                    case 0xF:
                        pop(&regs.AF.r16);
                    break;
                }
                ticks = 3;
            }
            else if(l == 0xF){
                push(regs.PC.r16+1);
                uint16_t addr = 0x8 + (h-0xC)*0x10;
                regs.PC.r16 = addr;
                ticks = 4;
            }
            else if(l == 0x7){
                push(regs.PC.r16+1);
                uint16_t addr = 0x10 + (h-0xC)*0x10;    
                regs.PC.r16 = addr;
                ticks = 4;
            }
            // jmp
            else if(h <= 0xD){
                switch(l){
                    case 0x3:
                        uint16_t addr = mem->get(regs.PC.r16+1);
                        addr += mem->get(regs.PC.r16+2)*0x100;
                        regs.PC.r16 = addr;                        
                    break;
                }
            } else {
                if(l == 0x2 || l == 0x0){
                    // This is disgusting
                    uint16_t addr = (l == 0)?0xFF00+mem->get(++regs.PC.r16) : (0xFF00+regs.BC.hl.r8l);
                    ticks = (l == 0)?3:2;
                    if(h == 0xE) {
                        mem->set(regs.AF.hl.r8h, addr);
                    } else {
                        regs.AF.hl.r8h = mem->get(addr);
                    }
                    regs.PC.r16 += 1;
                } 
                if(l == 0x3){
                    IME = false;
                    regs.PC.r16 += 1;
                }
                if(l == 0xB){
                    IME = true;
                    regs.PC.r16 += 1;
                }
                if(l == 0xE){
                    uint8_t n = mem->get(++regs.PC.r16);
                    if(h == 0xF){
                        cmp(regs.AF.hl.r8h, n);
                    } else {
                        regs.AF.hl.r8h = regs.AF.hl.r8h ^ n;
                        if(regs.AF.hl.r8h == 0){
                            (*flags) &= (0x1<<0x7); // ZERO flag
                        }
                    }
                    ticks = 2;
                    regs.PC.r16 += 1;
                }
            }
        }

        tick(ticks);
    }
}

void CPU::push(uint16_t dat){
    regs.SP.r16-=1;
    mem->set(dat/0x10,regs.SP.r16);
    regs.SP.r16-=1;
    mem->set(dat%0x10,regs.SP.r16);
}

void CPU::pop(uint16_t* dat){
    (*dat) = mem->get(regs.SP.r16) * 0x10;
    regs.SP.r16+=1;
    (*dat) += mem->get(regs.SP.r16);
    regs.SP.r16+=1;
}

bool CPU::tick(uint8_t cpu_cycles){
    for(int i = 0; i < cpu_cycles; i++){
        clock->tick();
//        ppu->tick();
    }
    // any interrupt
    if(!mem->get_int(0xFF)){
        return false;        
    }


    push(regs.PC.r16);
    int interrupt = mem->get_int_num();
    if(IME && ((*IE) & (1 << interrupt)) > 0){
        std::cout << "Interrupt Found: " << interrupt << "\n";
        regs.PC.r16 = 0x40 + (interrupt * 8);
        mem->reset_int(pow(2, interrupt));
    }
    IME = false;
    
    return true;
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