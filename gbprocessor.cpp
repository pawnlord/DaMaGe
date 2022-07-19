#include "gbprocessor.h"
#include <SDL2/SDL.h>

void CPU::setbcdflags(uint8_t before, uint8_t operand, bool issub){
    setflag(6, issub);
    if(issub){  
        uint8_t ol = operand & 0x0F;
        uint8_t bl = before & 0x0F;
        setflag(5, ((bl - ol) < 0));
    } else {
        uint8_t ol = operand & 0x0F;
        uint8_t bl = before & 0x0F;
        setflag(5, (bl + ol) > 0xF);
    }
}
void CPU::setbcddir(bool ishc, bool issub){
    (*flags) = ((*flags) & ~(1<<6)) | ((issub?1:0)<<6);
    (*flags) = ((*flags) & ~(1<<5)) | ((ishc?1:0)<<5);
}

void CPU::clearbcd(){
    (*flags) = ((*flags) & ~(0b11<<5)); 
}

gbreg& gbreg::operator=(const uint16_t a){
    this->r16 = a;
    return *this;
}

CPU::CPU(Memory *mem, Clock*clock){
    this->mem = mem;
    this->clock = clock;
    ppu = new PPU(mem);
    regs.PC = 0x100;
    regs.AF.hl.r8h = 0x01;
    regs.BC = 0x0013;
    regs.DE = 0x00D8;
    regs.HL = 0x014D;
    regs.SP = 0xFFFE;
    flags = &regs.AF.hl.r8l;
    (*flags) = 0x80;
    IME = true;
    halt_flag = false;
    IE = mem->getref(0xFFFF);
    IF = mem->getref(0xFF0F);
    tempp = mem->raw_mem;
}
PPU *CPU::getppu(){
    return ppu;
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
        r->r16 = (uint16_t)val;
    }
}

bool CPU::getflag(int flag){
    return ((*flags) & (0x1<<flag)) > 0;
}
void CPU::setflag(int flag, bool set){
    *flags = ((*flags) & ~(1<<flag)) | ((set)?1<<flag:0);
}

void CPU::cmp(int a, int b){
    setbcdflags(a, b, true);
    if(a > b){
        setflag(4, false);
        setflag(7, false);
    } else if(a == b){
        setflag(4, false);
        setflag(7, true);
    } else if(a < b){
        setflag(4, true);
        setflag(7, false);
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
    IME = true;
}
void CPU::run(){
    static uint32_t temp = 0;
    uint16_t address = -1;
    while(true){
        uint8_t opcode = mem->get(regs.PC.r16);
        uint8_t h = opcode/0x10, l = opcode%0x10; // get highest bit
        if(mem->is_change_speed()){
            clock->set_speed(change_speed);
        } else {
            clock->set_speed(DEFAULT_SPEED);
        }
        
        temp += 1;
        if(debug){
            cycles_to_run -= 1;
            if(cycles_to_run == 0 || regs.PC.r16 == address){
                std::string inp;
                print_info();
                std::cout << regs.PC.r16 << " " << (int)opcode << "(" << (int)mem->get(regs.PC.r16+1) << "," << (int)mem->get(regs.PC.r16+2) <<  ")" << "\n";
                std::cin >> inp;
                if(inp.substr(0,5) == "break"){
                    address = std::strtol(inp.substr(5, inp.length()).c_str(), NULL, 16);
                    std::cout << address << std::endl;
                    cycles_to_run = 1;
                } else {
                    cycles_to_run = std::atoi(inp.c_str());
                }
                if(cycles_to_run == -1){
                    cycles_to_run = 1;
                    std::cout << "temp" << temp << "\n";
                    mem->dump();
                    std::ofstream oftest ("mbc.dmp", std::ofstream::binary);
                    oftest.write((char*)(mem->mbc->full), 0x10000);
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
                char i8 = (char)(mem->get(++regs.PC.r16));
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
            } // 0x879: tilemap data
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
                uint8_t val, start;
                if(ishigh != FULL){
                    start = getval(firstarg, ishigh);
                    val = start+1;
                    setval(firstarg, ishigh, val);    
                } else {
                    start = mem->get(firstarg->r16);
                    val = start+1;
                    mem->set(val, firstarg->r16);
                }
                setbcdflags(start, 1, false);
                regs.PC.r16 += 1;
                ticks = 1;
                setflag(7, val == 0);
            }
            if(l == 0x5 || l == 0xD){
                uint8_t val, start;
                if(ishigh != FULL){
                    start = getval(firstarg, ishigh);
                    val = start-1;
                    setval(firstarg, ishigh, val);    
                } else {
                    start = mem->get(firstarg->r16);
                    val = start-1;
                    mem->set(val, firstarg->r16);
                }    
                setbcdflags(start, 1, true);
                regs.PC.r16 += 1;
                ticks = 1;
                setflag(7, val == 0);
            }
            if(l == 0x6 ||  l == 0xE){
                regs.PC.r16 += 1;
                uint8_t val = mem->get(regs.PC.r16);
                if(ishigh != FULL){
                    setval(firstarg, ishigh, val);
                } else {
                    mem->set(val, firstarg->r16);
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
                    regs.AF.hl.r8h = (regs.AF.hl.r8h & ~0x1) | lastbit; 
                    setflag(4, lastbit>0);
                    setflag(7, false);
                    setbcddir(false, false);
                } else if(h == 0x1){
                    uint8_t lastbit = (regs.AF.hl.r8h & 0x80) >> 3; // get last bit to keep
                    uint8_t carrybit = ((*flags) & 0x10) >> 4; 
                    regs.AF.hl.r8h <<= 1;
                    regs.AF.hl.r8h = (regs.AF.hl.r8h & ~0x1) | carrybit;     
                    setflag(4, lastbit>0);
                    setflag(7, false);
                    setbcddir(false, false);
                } else if (h == 0x2){
                    uint8_t *A = &regs.AF.hl.r8h;
                    int val = *A;
                    // credit: https://forums.nesdev.org/viewtopic.php?p=196282&sid=42ad7984120d6eb63b476586dc5eccaa#p196282
                    if (!getflag(6)) { 
                       if (getflag(4) || val  > 0x99) { 
                            val += 0x60; 
                            setflag(4, true);
                        }
                        if (getflag(5) || (val & 0x0f) > 0x09) { 
                            val += 0x6; 
                        }
                    } else {
                        if (getflag(4)) {
                            val -= 0x60; 
                        } 
                        if (getflag(5)) { 
                            val -= 0x6; 
                        }
                    }
                    (*A) = val & 0xFF;
                    setflag(7, (*A) == 0);
                    setflag(5, false);
                } else if (h == 0x3){
                    setflag(4, true);
                    setbcddir(false, false);
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
                    uint16_t addr = regs.PC.r16+offset;
                    ticks = 2;
                    if(h == 0x1 || (h == 0x2 && (getflag(0x7))) 
                                || (h == 0x3 && (getflag(0x4)))){ // JR
                        regs.PC.r16=addr;
                        ticks = 3;
                    }
                    regs.PC.r16+=1;
                }
            }
            if(l == 0x9){
                if(firstarg == &regs.AF){
                    firstarg = &regs.SP;
                }
                uint16_t val = firstarg->r16;
                setflag(4, (regs.HL.r16) + val > 0xFFFF);
                setbcddir((regs.HL.r16 & 0xFFF) + (val & 0xFFF) > 0xFFF, false);
                regs.HL.r16 = regs.HL.r16 + val;
                regs.PC.r16+=1;
            }
            if(l == 0xA){
                if(firstarg == &regs.AF){
                    ishigh = FULL;
                    firstarg = &regs.HL;
                }    
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
                if(firstarg == &regs.AF){
                    firstarg = &regs.SP;
                }
                firstarg->r16 = firstarg->r16-1;
                regs.PC.r16 += 1;
                ticks = 2;
            }
            if(l == 0xF){
                if(h == 0x0){
                    uint8_t lastbit = (regs.AF.hl.r8h & 0x1) << 7; // get last bit to keep
                    regs.AF.hl.r8h >>= 1;
                    regs.AF.hl.r8h = (regs.AF.hl.r8h & ~0x80) | lastbit; 
                    setflag(4, lastbit > 0);
                    setflag(7, false);
                    setbcddir(false, false);
                } else if(h == 0x1){
                    uint8_t lastbit = (regs.AF.hl.r8h & 0x1); // get last bit to keep
                    uint8_t carrybit = ((*flags) & 0x10) << 3; 
                    setflag(4, lastbit > 0);
                    regs.AF.hl.r8h >>= 1;
                    regs.AF.hl.r8h = (regs.AF.hl.r8h & ~0x80) | carrybit;     
                    setflag(7, false);
                    setbcddir(false, false);
                } else if (h == 0x2){
                    uint8_t *A = &regs.AF.hl.r8h;
                    (*A) = (*A) ^ 0xFF;
                    setbcddir(true, true);
                } else if (h == 0x3){
                    (*flags) ^= (1<<0x4);                    
                    setbcddir(false, false);
                }
                ticks = 1;
                regs.PC.r16 += 1;
            }
        } 
        
        // Load
        else if(h >= 0x4 && h <= 0x7){
            regs.PC.r16 += 1;
            if(opcode == 0x76){
                halt();
            } else {
                regtype_e ftype, stype;
                gbreg *farg, *sarg;
                farg = get_first_arg(opcode, &ftype);
                sarg = get_last_arg(l%0x8, &stype);
                if(l == 0x6 || l == 0xE || h == 0x7){
                    ticks = 2;
                } else{
                    ticks = 1;
                }
                uint8_t val = (stype == FULL)? mem->get(sarg->r16) : getval(sarg, stype);
                
                if(ftype != FULL){
                    setval(farg, ftype, val);
                } else {
                    mem->set(val, farg->r16);
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
            uint8_t val = (ishigh==FULL)?mem->get(arg->r16):getval(arg, ishigh);
            if(l <= 7){
                if(h == 0x8){
                    setflag(4, (*A) + val > 0xFF);
                    setflag(7, (((*A) + val) & 0xFF) == 0);
                    setbcdflags((*A), val, false);
                    (*A) = (*A) + val;
                } else if(h == 0x9){
                    setflag(4, (*A) - val < 0);
                    setflag(7, (((*A) - val) & 0xFF) == 0);
                    setbcdflags((*A), val, true);
                    (*A) = (*A) - val;
                } else if(h == 0xA){
                    setflag(7, ((*A) & val) == 0);
                    setbcddir(true, false);
                    setflag(4, false);
                    (*A) = (*A) & val;
                } else if(h == 0xB){
                    setflag(7, ((*A) | val) == 0);
                    setflag(4, false);
                    setbcddir(false, false);
                    (*A) = (*A) | val;
                }
            } else {
                if(h == 0x8){
                    uint8_t cy = getflag(0x4)?1:0;
                    setflag(4, (*A) + val + cy > 0xFF);
                    setflag(7, (((*A) + val + cy) & 0xFF) == 0);
                    setbcddir(((*A & 0xF) + (val & 0xF) + cy) > 0xF, false);
                    (*A) = (*A) + val + cy;
                    
                } else if(h == 0x9){
                    uint8_t cy = getflag(0x4)?1:0;
                    setflag(4, ((*A) - val - cy < 0));
                    setflag(7, (((*A) - val - cy) & 0xFF) == 0);
                    setbcddir(((*A & 0xF) - (val & 0xF) - cy) < 0, true);
                    (*A) = (*A) - val - cy;
                } else if(h == 0xA){
                    setflag(7, ((*A) ^ val) == 0);
                    setbcddir(false, false);
                    setflag(4, false);
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
                uint16_t addr = (h-0xC)*0x10;    
                regs.PC.r16 = addr;
                ticks = 4;
            }
            // jmp
            else if(h <= 0xD){
                if(l == 0x2 || l == 0xA){
                    bool succ = jmp((h == 0xC)?CON_ZERO:CON_CARRY, false, l==0x2);
                    ticks = succ?4:3;
                    if(!succ){regs.PC.r16 += 1;}
                }
                if(l == 0x3){
                    jmp(CON_NONE, false, false);                        
                    ticks = 4;
                }
                if (l == 0x4 || l == 0xC){
                    bool succ = jmp((h == 0xC)?CON_ZERO:CON_CARRY, true, l==0x4);
                    ticks = succ?6:3;
                    if(!succ){regs.PC.r16 += 1;}
                }
                if(l == 0x6){
                    uint8_t* A = &regs.AF.hl.r8h;
                    uint8_t u8 = mem->get(++regs.PC.r16), carry, zero;
                    uint8_t start = (*A);
                    if(h == 0xC){
                        carry = ((*A) + u8 > 0xFF) << 0x4;
                        zero = ((((*A) + u8) & 0xFF) == 0) << 0x7;
                        (*A) = (*A) + u8;
                    } else {
                        carry = ((*A) - u8 < 0) << 0x4;
                        zero = ((((*A) - u8) & 0xFF) == 0) << 0x7;
                        (*A) = (*A) - u8;    
                    }
                    setbcdflags(start, u8, h==0xD);
                    (*flags) = ((*flags) & (~0x10) & (~0x80)) | carry | zero;
                    regs.PC.r16 += 1;
                    ticks = 2;
                }
                if(l == 0 || l == 0x8){
                    bool succ = ret((h==0xC)?CON_ZERO:CON_CARRY, l==0);
                    ticks = (succ)?5:2;
                    if(!succ){regs.PC.r16 += 1;}
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
                if(l == 0xE){
                    uint8_t* A = &regs.AF.hl.r8h;
                    uint8_t cy = getflag(0x4)?1:0;
                    uint8_t u8 = mem->get(++regs.PC.r16);
                    if(h == 0xC){
                        setflag(4, (*A) + u8 + cy > 0xFF);
                        setflag(7, (((*A) + u8 + cy) & 0xFF) == 0);
                        setbcddir(((*A & 0xF) + (u8 & 0xF) + cy) > 0xF, false);
                        (*A) = (*A) + u8 + cy;
                    } else {
                        setflag(4, (*A) - u8 - cy < 0);
                        setflag(7, (((*A) - u8 - cy) & 0xFF) == 0);
                        setbcddir(((*A & 0xF) - (u8 & 0xF) - cy) < 0, true);
                        (*A) = (*A) - u8 - cy;    
                    }
                    regs.PC.r16 += 1;
                    ticks = 2;
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
                    uint8_t u8 = mem->get(++regs.PC.r16);
                    if(h == 0xF){
                        regs.AF.hl.r8h = regs.AF.hl.r8h | u8;
                        setflag(7,regs.AF.hl.r8h == 0);
                        setflag(4, false);
                        setbcddir(false, false);
                    } else {
                        regs.AF.hl.r8h = regs.AF.hl.r8h & u8;
                        setflag(7,regs.AF.hl.r8h == 0);
                        setflag(4, false);
                        setbcddir(true, false);
                    }
                    ticks = 2;
                    regs.PC.r16 += 1;
                }
                if(l == 0x8){
                    int8_t i8 = (int8_t)mem->get(++regs.PC.r16);
                    setflag(7, false);
                    setflag(5, (regs.SP.r16 & 0xF) + (((uint16_t)i8) & 0xF) > 0xF);
                    setflag(4, (regs.SP.r16 & 0xFF) + (((uint16_t)i8) & 0xFF) > 0xFF);
                    setflag(6, false);
                    if(h == 0xE){
                        regs.SP.r16 += i8; 
                        ticks = 4;
                    } else {
                        regs.HL.r16 = regs.SP.r16 + i8;
                        ticks = 3;
                    }
                    regs.PC.r16 += 1;
                }
                if(l == 0x9){
                    if(h == 0xE){
                        regs.PC.r16 = regs.HL.r16; 
                        ticks = 1;
                    } else {
                        regs.SP.r16 = regs.HL.r16; 
                        ticks = 2;
                        regs.PC.r16+=1;
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
                    uint8_t u8 = mem->get(++regs.PC.r16);
                    if(h == 0xF){
                        cmp(regs.AF.hl.r8h, u8);
                    } else {
                        regs.AF.hl.r8h = regs.AF.hl.r8h ^ u8;
                        setflag(7,regs.AF.hl.r8h == 0);
                        setflag(4,false);
                        setbcddir(false, false);
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
    
    uint8_t* arg;
    if(type == FULL){
        arg = mem->getref(argreg->r16);
    } else if(type == HIGH){
        arg = &argreg->hl.r8h;
    } else if(type == LOW){
        arg = &argreg->hl.r8l;
    }
    
    int ticks = (type == FULL)?4:2;
    if(opcode < 0x40){
        bool isleft = (opcode/8)%2 == 0;
        if(isleft){
            uint8_t lastbit;
            uint8_t carrybit;
            uint8_t firstbit, zero;
            uint8_t h, l;
            switch(opcode/0x10){
                case 0:
                    lastbit = ((*arg) & 0x80) >> 7; // get last bit to keep
                    (*arg) <<= 1;
                    (*arg) = ((*arg) & ~0x1) | lastbit; 
                    setflag(4, lastbit>0);
                    setflag(7, ((*arg) & 0xFF) == 0);
                break;
                case 1:
                    lastbit = ((*arg) & 0x80); // get last bit to keep
                    carrybit = ((*flags) & 0x10) >> 4; 
                    (*arg) <<= 1;
                    (*arg) = ((*arg) & ~0x1) | carrybit;     
                    setflag(4, lastbit>0);
                    setflag(7, ((*arg) & 0xFF) == 0);
                break;
                case 2:
                    lastbit = ((*arg) & 0x80); // get last bit to keep
                    (*arg) <<= 1;
                    (*arg) &= ~0x1; 
                    setflag(4, lastbit>0);
                    setflag(7, ((*arg) & 0xFF) == 0);
                break;
                case 3:
                    h = (*arg)/0x10;
                    l = (*arg)%0x10;
                    (*arg) = h + l*0x10;
                    setflag(4, false);
                    setflag(7, ((*arg) & 0xFF) == 0);
                break;
            }
        } else {
            uint8_t lastbit, carrybit, firstbit, zero;
            switch(opcode/0x10){
                case 0:
                    lastbit = ((*arg) & 0x1) << 7; // get last bit to keep
                    (*arg) >>= 1;
                    (*arg) = ((*arg) & ~0x80) | lastbit; 
                    setflag(4, lastbit>0);
                    setflag(7, ((*arg) & 0xFF) == 0);
                break;
                case 1:
                    lastbit = ((*arg) & 0x1) << 4; 
                    carrybit = ((*flags) & 0x10) << 3; 
                    setflag(4, lastbit>0);
                    (*arg) >>= 1;
                    (*arg) = ((*arg) & ~0x80) | carrybit;
                    setflag(7, ((*arg) & 0xFF) == 0);
                break;
                case 2:
                    lastbit = ((*arg) & 0x80); // get last bit to keep
                    firstbit = ((*arg) & 0x1) << 4;
                    (*arg) >>= 1;
                    (*arg) = ((*arg) & ~0x80) | lastbit; 
                    setflag(4, firstbit>0);
                    setflag(7, ((*arg) & 0xFF) == 0);
                break;
                case 3:
                    firstbit = ((*arg) & 0x1) << 4; 
                    (*arg) >>= 1;
                    (*arg) = ((*arg) & ~0x80); 
                    setflag(4, firstbit>0);
                    setflag(7, ((*arg) & 0xFF) == 0);
                break;
            }
        }
        setbcddir(false, false);
    } else {
        uint8_t bit = (opcode%0x40)/0x8;
        if(opcode < 0x80){
            bool test = ((*arg) & (1<<bit)) == 0;
            setflag(7, test);
            if(ticks == 4){
                ticks = 3;
            }
            setbcddir(true, false);
        } else if(opcode < 0xC0){
            (*arg) &= ~(1<<bit);
        } else if(opcode < 0x100){
            (*arg) |= (1<<bit);
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
    (*flags) &= 0xF0; // Make sure flags doesn't change   
}

bool CPU::tick(uint8_t cpu_cycles){
    
    for(int i = 0; i < cpu_cycles; i++){
        clock->tick();
        ppu->tick();
        mem->tick();
    }
    // any interrupt
    if(!mem->get_int(0xFF)){
        return false;        
    }


    int interrupt = mem->get_int_num();
    if(IME && ((*IE) & (1 << interrupt)) > 0){
        push(regs.PC.r16);
        regs.PC.r16 = 0x40 + (interrupt * 8);
        mem->reset_int(1<<interrupt);
        IME = false;
    }
    
    return true;
}


Clock::Clock(Memory *mem){
    this->mem = mem;
    this->timereg = mem->timereg;
    count = 0;
    last_time = std::clock();
}

void Clock::set_speed(int ops_per_mill){
    this->ops_per_mill = ops_per_mill;
}
void CPU::set_change_speed(double ops_mult){
    change_speed = DEFAULT_SPEED * ops_mult;
}

// 4 Clock ticks at once
void Clock::tick(){
    count += 4;
    dot_diff += 1;
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
    }
    if(count % 256 == 0){
        timereg->DIV += 1;
    }
    count %= 4194304; // cycles per second 
    if(dot_diff >= ops_per_mill){
        int diff = 0;
        while(diff == 0){
            diff = (std::clock() - last_time) / (double)(CLOCKS_PER_SEC / 1000);
        }
        last_time = std::clock();
        dot_diff = 0;
    }
}
void CPU::print_info(){
    std::cout << std::hex << "A: " << (int)(regs.AF.hl.r8h) << " F: " << (int)(regs.AF.hl.r8l) << " (AF " << regs.AF.r16 << ")\n"; 
    std::cout << std::hex << "B: " << (int)(regs.BC.hl.r8h) << " C: " << (int)(regs.BC.hl.r8l) << " (BC " << regs.BC.r16 << "=" << (int)mem->get(regs.BC.r16) << ")\n"; 
    std::cout << std::hex << "D: " << (int)(regs.DE.hl.r8h) << " E: " << (int)(regs.DE.hl.r8l) << " (DE " << regs.DE.r16 << "=" << (int)mem->get(regs.DE.r16) << ")\n"; 
    std::cout << std::hex << "H: " << (int)(regs.HL.hl.r8h) << " L: " << (int)(regs.HL.hl.r8l) << " (HL " << regs.HL.r16 << "=" << (int)mem->get(regs.HL.r16) << ")\n"; 
    std::cout << "PC: " << regs.PC.r16 << " SP:" << regs.SP.r16 << "\n"; 
    std::cout << "[" << (((*flags)&1<<7)?"Z":"-") << (((*flags)&1<<6)?"N":"-") << (((*flags)&1<<5)?"H":"-") << (((*flags)&1<<4)?"C":"-") << "]" << std::endl; 
    std::cout << "Interrupt Info: IME=" << IME << ", IF=" << (int)mem->get(0xFF0F) << ", IE=" << (int)mem->get(0xFFFF) << "\n"; 
    std::cout << "Video Info: S=" << (int)ppu->getstatfull() << ", C=" << (int)ppu->getlcdcfull() << std::endl;
    mem->mbc->print_info();
}
