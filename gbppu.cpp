#include "gbppu.h"

void PPU::tick(){
    dots+=1;
    if(dots % 456 == 0){
        dots = 0;
        (*LY)+=1;
        // std::cout << (int)(*LY) << std::endl;
        mode = ((*LY)>143)?M1:M2;
        
        if(((*STAT) & 0x1<<(3+mode)) > 0){ // switch mode 
            if(mem->get_int_enabled(0x2))
                mem->req_int(0x2);
        }
        
        // Interrupts
        if((*LY) == (*LYC)){
            (*STAT) != 1<<0x2; // set LYC=LY
            if(((*STAT) & 0x1<<6) > 0){
                if(mem->get_int_enabled(0x2))
                    mem->req_int(0x2);
            }
        } else {
            (*STAT) = (*STAT) & ~(1<<0x2);
        }
        if((*LY) == 154){
            (*LY) = 0;
            mode = M2;
        }
    }
    (*STAT) = ((*STAT) & ~(0b11)) | mode;
    if(mode == M2){
        updt_oamscan();
    } else if (mode == M3){
        updt_drawpxl();
    }
}

void PPU::updt_oamscan(){

}
void PPU::updt_drawpxl(){

}

PPU::PPU(Memory *mem){
    this->mem = mem;
    this->tile_ref = (uint16_t*)mem->getref(0x8000);
    this->LCDC = mem->getref(0xFF40);
    this->STAT = mem->getref(0xFF41);
    this->OAM = (object_t*)mem->getref(0xFE00);
    this->mode = M2; 
    this->LY = mem->getref(kLY);
    this->LYC = mem->getref(kLYC);
}

bool PPU::getlcdc(int val){
    int temp = (int)(pow(2, val));
    if (temp & (*LCDC) > 0){
        return true;
    }
    return false;
}
