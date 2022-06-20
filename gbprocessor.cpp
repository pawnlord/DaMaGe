#include "gbprocessor.h"

gbreg& gbreg::operator=(const uint16_t a){
    this->r16 = a;
    return *this;
}

CPU::CPU(Memory *mem, Clock*clock){
    this->mem = mem;
    this->clock = clock;
    ppu = new PPU(mem);
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

bool CPU::getflag(int flag){
    return ((*flags) & (0x1<<flag)) > 0;
}

void CPU::cmp(int a, int b){
    int cmpn = a - b;
    if(cmpn > 0){
        (*flags) &= ~(0x1<<0x7); // ZERO flag 
        (*flags) &= ~(0x1<<0x4); // CARRY flag 
    } else if(cmpn == 0){
        (*flags) |= (0x1<<0x7); // ZERO flag
        (*flags) &= ~(0x1<<0x4); // CARRY flag 
    } else {
        (*flags) &= ~(0x1<<0x7); // ZERO flag 
        (*flags) |= (0x1<<0x4); // CARRY flag
    }
}

bool CPU::jmp(condition_e con, bool isCall, bool isNot){
    if(con != CON_NONE){
        bool flag = (con == CON_CARRY)?getflag(0x4):getflag(0x7);
        if(isNot) {flag = !flag;}
        if(!flag){
            regs.PC.r16+=2;
            return false;
        }
    }
    uint16_t addr = mem->get(++regs.PC.r16);
    addr += mem->get(++regs.PC.r16)*0x100;
    if(isCall){
        push(regs.PC.r16+1);
    }
    regs.PC.r16 = addr;
    return true;
}

bool CPU::ret(condition_e con, bool isNot){
    if(con != CON_NONE){
        bool flag = (con == CON_CARRY)?getflag(0x4):getflag(0x7);
        if(isNot) {flag = !flag;}
        if(!flag){
            return false;
        }
    }
    pop(&regs.PC.r16);
    return true;
}
void CPU::reti(){
    ret(CON_NONE, false);
    IME = 1;
}
void CPU::run(){
    static uint32_t temp = 0;
    while(true){
        if(mem->mbc.full[0xFFF3] == 0){
            std::cout << "breakpoint\n";
        }
        uint8_t opcode = mem->get(regs.PC.r16);
        uint8_t h = opcode/0x10, l = opcode%0x10; // get highest bit
        if(debug){
            temp += 1;
            cycles_to_run -= 1;
            if(cycles_to_run == 0){
                std::cout << regs.PC.r16 << " " << (int)opcode << "\n";
                std::cin >> cycles_to_run;
                if(cycles_to_run == -1){
                    cycles_to_run = 1;
                    std::cout << "temp" << temp << "\n";
                    mem->dump();
                    std::ofstream oftest ("mbc.dmp", std::ofstream::binary);
                    oftest.write((char*)(mem->mbc.full), 0x10000);
                    oftest.close();
                }
            }
        }
        int ticks = 1;
        // misc.
        if(opcode == 0x0){
            regs.PC.r16 += 1;
        }
        else if(h <= 0x3){
            regtype_e ishigh;
            gbreg* firstarg = get_first_arg(opcode, &ishigh);
            if(l == 0x0){
                char i8 = (int)(mem->get(++regs.PC.r16));
                uint16_t addr = regs.PC.r16 + i8;
                ticks = 2;
                if(((h == 0x2 && !(getflag(0x7)))) ||
                    (h == 0x3 && !(getflag(0x4)))){
                    regs.PC.r16 = addr;
                    ticks = 3;
                } 
                regs.PC.r16 += 1;
            }
            if(l == 0x1){
                if(ishigh == FULL){
                    firstarg = &regs.SP;
                }
                uint16_t u16 = mem->get(++regs.PC.r16);
                u16 += mem->get(++regs.PC.r16)*0x100;
                firstarg->r16 = u16;
                regs.PC.r16 += 1;
                ticks = 3;
            }
            if(l == 0x2){
                mem->set(regs.AF.hl.r8h, firstarg->r16);
                regs.PC.r16 += 1;
                ticks = 2;
                if(firstarg == &regs.HL){
                    if(ishigh == FULL){
                        regs.HL.r16 -= 1;
                    } else {
                        regs.HL.r16 += 1;
                    }
                }
            }
            if(l == 0x3){
                if(ishigh == FULL){
                    firstarg = &regs.SP;
                }
                setval(firstarg, FULL, getval(firstarg, FULL)+1);    
                regs.PC.r16 += 1;
                ticks = 2;
            }
            if(l == 0x4 || l == 0xC){
                uint8_t val;
                if(ishigh != FULL){
                    val = getval(firstarg, ishigh)+1;
                    setval(firstarg, ishigh, val);    
                } else {
                    val = mem->get(firstarg->r16)+1;
                    mem->set(val, firstarg->r16);
                }
                regs.PC.r16 += 1;
                ticks = 1;
                (*flags) = (val == 0)?((*flags) | (0x1<<0x7)):((*flags) & ~(0x1<<0x7));
            }
            if(l == 0x5 || l == 0xD){
                uint8_t val;
                if(ishigh != FULL){
                    val = getval(firstarg, ishigh)-1;
                    setval(firstarg, ishigh, val);    
                } else {
                    val = mem->get(firstarg->r16)-1;
                    mem->set(val, firstarg->r16);
                }    
                regs.PC.r16 += 1;
                ticks = 1;
                (*flags) = (val == 0)?((*flags) | (0x1<<0x7)):((*flags) & ~(0x1<<0x7));
            }
            if(l == 0x6 ||  l == 0xE){
                regs.PC.r16 += 1;
                uint8_t val = mem->get(regs.PC.r16);
                if(ishigh != FULL){
                    setval(firstarg, ishigh, val);
                } else {
                    mem->set(val, getval(firstarg, ishigh));
                }
                regs.PC.r16 += 1;
                ticks = 2;
                if(ishigh == FULL){
                    ticks = 3;
                }
            }
            if(l == 0x7){
                if(h == 0x0){
                    uint8_t lastbit = (regs.AF.hl.r8h & 0x80) >> 7; // get last bit to keep
                    regs.AF.hl.r8h <<= 1;
                    regs.AF.hl.r8h == (regs.AF.hl.r8h & ~0x80) | lastbit; 
                    (*flags) = ((*flags) & ~0x10) | (lastbit<<4);
                } else if(h == 0x1){
                    uint8_t lastbit = (regs.AF.hl.r8h & 0x80) >> 3; // get last bit to keep
                    uint8_t carrybit = ((*flags) & 0x10) >> 3; 
                    (*flags) = ((*flags) & ~0x10) | lastbit;
                    regs.AF.hl.r8h <<= 1;
                    regs.AF.hl.r8h == (regs.AF.hl.r8h & ~0x1) | carrybit;     
                }
                ticks = 1;
                regs.PC.r16 += 1;
            }
            if(l == 0x8){
                if(h == 0x0){
                    uint16_t addr = mem->get(++regs.PC.r16);
                    addr += mem->get(++regs.PC.r16)*0x100;
                    mem->set(regs.SP.hl.r8l,addr);
                    mem->set(regs.SP.hl.r8h,addr+1);
                    ticks = 5;
                    regs.PC.r16 += 1;    
                } else {
                    int8_t offset = (mem->get(++regs.PC.r16));
                    uint16_t addr = regs.PC.r16;
                    ticks = 2;
                    if(h == 0x1 || (h == 0x2 && (getflag(0x7))) 
                                || (h == 0x3 && (getflag(0x7)))){ // JR
                        regs.PC.r16=addr;
                        ticks = 3;
                    }
                    regs.PC.r16+=1;
                }
            }
            if(l == 0x9){
                if(ishigh == FULL){
                    firstarg = &regs.SP;
                }
                uint16_t val = getval(firstarg, FULL);
                uint8_t carry = ((regs.HL.r16) + val < 0) << 0x4;
                uint8_t zero = ((regs.HL.r16) + val == 0) << 0x7;
                (*flags) = ((*flags) & (~0x10) & (~0x80)) | carry | zero;
                regs.HL.r16 = regs.HL.r16 + val;
                regs.PC.r16+=1;
            }
            if(l == 0xA){
                regs.AF.hl.r8h = mem->get(firstarg->r16);
                regs.PC.r16 += 1;
                if(firstarg == &regs.HL){
                    if(ishigh == FULL){
                        regs.HL.r16 -= 1;
                    } else {
                        regs.HL.r16 += 1;
                    }
                }
                //std::cout << regs.HL.r16 << std::endl;
            }
            if(l == 0xB){
                if(ishigh == FULL){
                    firstarg = &regs.SP;
                }
                setval(firstarg, FULL, getval(firstarg, FULL)-1);    
                regs.PC.r16 += 1;
                ticks = 2;
            }
            if(l == 0xF){
                if(h == 0x0){
                    uint8_t lastbit = (regs.AF.hl.r8h & 0x1) << 7; // get last bit to keep
                    regs.AF.hl.r8h >>= 1;
                    regs.AF.hl.r8h == (regs.AF.hl.r8h & ~0x80) | lastbit; 
                    (*flags) = ((*flags) & ~0x10) | (lastbit>>3);
                } else if(h == 0x1){
                    uint8_t lastbit = (regs.AF.hl.r8h & 0x1) << 4; // get last bit to keep
                    uint8_t carrybit = ((*flags) & 0x10) << 4; 
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
                regtype_e ftype, stype;
                gbreg *farg, *sarg;
                farg = get_first_arg(opcode, &ftype);
                sarg = get_last_arg(l%0x8, &stype);
                if(l == 0x6 || l == 0xE){
                    ticks = 2;
                } else{
                    ticks = 1;
                }
                uint8_t val = (stype == FULL)? mem->get(getval(sarg, FULL)) : getval(sarg, stype);
                if(ftype != FULL){
                    setval(farg, ftype, val);
                } else {
                    mem->set(val, getval(farg, ftype));
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
            int val = (ishigh==FULL)?mem->get(getval(arg, FULL)):(int)getval(arg, ishigh);
            if(l <= 7){
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
                    uint8_t zero = ((*A) & val == 0) << 0x7;
                    (*flags) = ((*flags) & (~0x80)) | zero;
                    (*A) = (*A) & val;
                } else if(h == 0xB){
                    uint8_t zero = ((*A) | val == 0) << 0x7;
                    (*flags) = ((*flags) & (~0x80)) | zero;
                    (*A) = (*A) | val;
                }
            } else {
                if(h == 0x8){
                    uint8_t cy = getflag(0x4)?1:0;
                    uint8_t carry = ((*A) + val + cy < 0) << 0x4;
                    uint8_t zero = ((*A) + val + cy == 0) << 0x7;
                    (*flags) = ((*flags) & (~0x10) & (~0x80)) | carry | zero;
                    (*A) = (*A) + val + cy;
                } else if(h == 0x9){
                    uint8_t cy = getflag(0x4)?1:0;
                    uint8_t carry = ((*A) - val - cy < 0) << 0x4;
                    uint8_t zero = ((*A) - val - cy == 0) << 0x7;
                    (*flags) = ((*flags) & (~0x10) & (~0x80)) | carry | zero;
                    (*A) = (*A) - val - cy;
                } else if(h == 0xA){
                    uint8_t zero = ((*A) ^ val == 0) << 0x7;
                    (*flags) = ((*flags) & (~0x80)) | zero;
                    (*A) = (*A) ^ val;
                } else if(h == 0xB){
                    cmp((*A), val);
                }
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
                regs.PC.r16 += 1;
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
                regs.PC.r16 += 1;
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
                if(l == 0x2 || l == 0xA){
                    bool succ = jmp((h == 0xC)?CON_ZERO:CON_CARRY, false, l==0x2);
                    ticks = succ?4:3;
                }
                if(l == 0x3){
                    jmp(CON_NONE, false, false);                        
                    ticks = 4;
                }
                if (l == 0x4 || l == 0xC){
                    bool succ = jmp((h == 0xC)?CON_ZERO:CON_CARRY, true, l==0x4);
                    ticks = succ?6:3;
                }
                if(l == 0 || l == 0x8){
                    bool succ = ret((h==0xC)?CON_ZERO:CON_CARRY, l==0);
                    ticks = (succ)?5:2;
                }
                if(l == 0x9){
                    if(h == 0xC){
                        ret(CON_NONE, false);
                    } else {
                        reti();
                    }
                    ticks = 4;
                }
                if(l == 0xB){
                    ticks = prefixop(mem->get(++regs.PC.r16));
                    regs.PC.r16+=1;
                }
                if(l == 0xD){
                    jmp(CON_NONE, true, false);                        
                    ticks = 6;
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
                if(l == 0x6){
                    uint8_t n = mem->get(++regs.PC.r16);
                    if(h == 0xF){
                        regs.AF.hl.r8h = regs.AF.hl.r8h | n;
                        if(regs.AF.hl.r8h == 0){
                            (*flags) &= (0x1<<0x7); // ZERO flag
                        }
                    } else {
                        regs.AF.hl.r8h = regs.AF.hl.r8h & n;
                        if(regs.AF.hl.r8h == 0){
                            (*flags) &= (0x1<<0x7); // ZERO flag
                        }
                    }
                    ticks = 2;
                    regs.PC.r16 += 1;
                }
                if(l == 0x9){
                    if(h == 0xE){
                        regs.PC.r16 = regs.HL.r16; 
                        ticks = 1;
                    } else {
                        regs.SP.r16 = regs.HL.r16; 
                        ticks = 2;
                    }
                }
                if(l == 0xA){
                    uint16_t addr = mem->get(++regs.PC.r16);
                    addr += mem->get(++regs.PC.r16)*0x100;    
                    if(h == 0xE){
                        mem->set(regs.AF.hl.r8h, addr);
                    } else  {
                        uint8_t val = mem->get(addr);
                        regs.AF.hl.r8h = val;
                    }
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

int CPU::prefixop(uint8_t opcode){
    regtype_e type;
    gbreg* argreg = get_last_arg((opcode%0x10)%0x8, &type);
    uint8_t* arg = (type == FULL)?mem->getref(argreg->r16):((type==HIGH)?&argreg->hl.r8h:&argreg->hl.r8l);
    std::cout << arg << "\n";
    int ticks = (type == FULL)?4:2;
    if(opcode < 0x40){
        bool isleft = (opcode/8)%2 == 0;
        if(isleft){
            uint8_t lastbit;
            uint8_t carrybit;
            uint8_t firstbit;
            uint8_t h, l;
            switch(opcode/0x10){
                case 0:
                    lastbit = ((*arg) & 0x80) >> 7; // get last bit to keep
                    (*arg) <<= 1;
                    (*arg) == ((*arg) & ~0x1) | lastbit; 
                    (*flags) = ((*flags) & ~0x10) | (lastbit<<4);
                break;
                case 1:
                    lastbit = ((*arg) & 0x80) >> 3; // get last bit to keep
                    carrybit = ((*flags) & 0x10) << 4; 
                    (*flags) = ((*flags) & ~0x10) | lastbit;
                    (*arg) <<= 1;
                    (*arg) == ((*arg) & ~0x1) | carrybit;     
                break;
                case 2:
                    lastbit = ((*arg) & 0x80) >> 3; // get last bit to keep
                    (*arg) <<= 1;
                    (*arg) &= ~0x1; 
                    (*flags) = ((*flags) & ~0x10) | lastbit;
                break;
                case 3:
                    h = (*arg)/0x100;
                    l = (*arg)%0x100;
                    (*arg) = h + l*0x100;
                break;
            }
        } else {
            uint8_t lastbit;
            uint8_t carrybit;
            uint8_t firstbit;
            switch(opcode/0x10){
                case 0:
                    lastbit = ((*arg) & 0x1) << 7; // get last bit to keep
                    (*arg) >>= 1;
                    (*arg) == ((*arg) & ~0x80) | lastbit; 
                    (*flags) = ((*flags) & ~0x10) | (lastbit>>3);
                break;
                case 1:
                    lastbit = ((*arg) & 0x1) << 4; // get last bit to keep
                    carrybit = ((*flags) & 0x10) << 4; 
                    (*flags) = ((*flags) & ~0x10) | lastbit;
                    (*arg) >>= 1;
                    (*arg) == ((*arg) & ~0x80) | carrybit;     
                break;
                case 2:
                    lastbit = ((*arg) & 0x80); // get last bit to keep
                    firstbit = ((*arg) & 0x1) << 4; // get last bit to keep
                    (*arg) >>= 1;
                    (*arg) == ((*arg) & ~0x80) | lastbit; 
                    (*flags) = ((*flags) & ~0x10) | (firstbit);
                break;
                case 3:
                    firstbit = ((*arg) & 0x1) << 4; // get last bit to keep
                    (*arg) >>= 1;
                    (*arg) == ((*arg) & ~0x80); 
                    (*flags) = ((*flags) & ~0x10) | (firstbit);
                break;
            }
        }
    } else {
        uint8_t bit = (opcode%0x40)/0x8;
        if(opcode < 0x80){
            int test = ((*arg) & (1<<bit)) > 0;
            (*flags) = ((*flags) & ~(1<<7)) | (test<<7); 
            if(ticks == 4){
                ticks = 3;
            }
        } else if(opcode < 0xC0){
            (*arg) |= (1<<bit);
        } else if(opcode < 0xF0){
            (*arg) &= ~(1<<bit);
        }
    }
    return ticks;
}

void CPU::push(uint16_t dat){
    regs.SP.r16-=1;
    mem->set(dat/0x100,regs.SP.r16);
    regs.SP.r16-=1;
    mem->set(dat%0x100,regs.SP.r16);
}

void CPU::pop(uint16_t* dat){
    (*dat) = mem->get(regs.SP.r16);
    regs.SP.r16+=1;
    (*dat) += mem->get(regs.SP.r16) * 0x100;
    regs.SP.r16+=1;
}

bool CPU::tick(uint8_t cpu_cycles){
    for(int i = 0; i < cpu_cycles; i++){
        clock->tick();
        ppu->tick();
    }
    // any interrupt
    if(!mem->get_int(0xFF)){
        return false;        
    }


    int interrupt = mem->get_int_num();
    if(IME && ((*IE) & (1 << interrupt)) > 0){
        push(regs.PC.r16);
        std::cout << "Interrupt Found: " << interrupt << "\n";
        regs.PC.r16 = 0x40 + (interrupt * 8);
        mem->reset_int(pow(2, interrupt));
    }
    IME = false;
    
    return true;
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