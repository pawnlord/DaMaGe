#include "gbppu.h"

bool object_t::operator==(const object_t& rhs){
    return (y == rhs.y) && (x == rhs.x) && (idx == rhs.idx) && (flag == rhs.flag);
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
    this->SCX = mem->getref(0xFF43);
    this->SCY = mem->getref(0xFF42);
    this->WX = mem->getref(0xFF4B);
    this->WY = mem->getref(0xFF4A);
}


void PPU::tick(){
    dots+=1;
    if(dots % 456 == 0){
        displayX = 0;
        lineobjs = 0;
        fetchedobjs.clear();
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
            totalobjs = 0;
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
    pxl_fetcher();
}


void PPU::init_drawpxl(){
    static std::queue<pixel_t> empty;
    std::swap(fgfifo, empty);
    std::swap(bgfifo, empty);

    // reset fetchX to a roundable number;
    uint8_t offset = (*SCX)&7;
    fetchX = (offset==0)?0:8-offset;
    displayX = fetchX;
}

void PPU::updt_oamscan(){
    static int objcount = 0;
    object_t obj = OAM[objcount];
    if((obj.y > (*LY) && obj.y < (*LY) + (getlcdc(2))?8:16 ) && lineobjs < 10 && totalobjs < 10){
        lineobjs++;
        totalobjs++;
        fetchedobjs.push_back(obj);
    }
    objcount++;
    if(objcount == 40){
        mode = M3;
        objcount = 0;
        init_drawpxl();
    }
}

void PPU::pxl_fetcher(){
        static enum state_e{GET_TILE = 0, GET_LOW, GET_HIGH, SLEEP, PUSH, SPRITE} state = GET_TILE;
    static enum spstate_e{ADVANCE, RETRIEVE, PUSH_SPRITE} spstate = ADVANCE;
    static bool isAdvance = false;
    static uint8_t tile;
    static bool isWin;
    static uint8_t tilehigh, tilelow;
    static object_t curr_sprite;
    // BG/Win
    if(state == GET_TILE) {
        isWin = (*LY) >= (*WY) && fetchX >= (*WX)-7;
        uint16_t tilemap, fetcherX, fetcherY;
        tilemap = 0x9800;
        if((isWin && getlcdc(6)) || (!isWin && getlcdc(3))){
            tilemap = 0x9C00;
        }
        if(!isWin){
            fetcherX = ((*SCX)+fetchX) / 8;
            fetcherY = ((*LY)+(*SCY)) / 8;
        } else {
            fetcherX = (fetchX - ((*WX) - 7)) / 8;
            fetcherY = ((*LY) - (*WY)) / 8;    
        }
        tile = mem->get(tilemap + (fetcherX + fetcherY*32)%0x400); // maybe>>>
        state = (state_e)(((int)state) + 1);
    } else if(state == GET_LOW){
        uint8_t row = (isWin)? ((*LY)-(*WY))&7: ((*LY)+(*SCY))&7;
        uint16_t tiledata = getlcdc(4)? 0x8000 + tile*16 + row*2: 0x9000 + ((int)tile)*16 + row*2;
        tilelow = mem->get(tiledata);
        state = (state_e)(((int)state) + 1);
    } else if(state == GET_HIGH){
        uint8_t row = (isWin)? ((*LY)-(*WY))&7: ((*LY)+(*SCY))&7;
        uint16_t tiledata = getlcdc(4)? 0x8000 + tile*16 + row*2 + (((*LY)/8) ) + 1 : 0x9000 + ((int)tile)*16 + row*2 + 1;
        tilehigh = mem->get(tiledata);
        state = (state_e)(((int)state) + 1);
    } else if(state == SLEEP){
        state = (state_e)(((int)state) + 1);
    } else if(state == PUSH){
        if(bgfifo.size() < 16){
            uint16_t fulltile = tilelow + (tilehigh*0x100);
            for(int i = 0; i < 8; i++){
                uint8_t pixel_raw = fulltile & (0b11 << (i*2));
                pixel_t pxl = {pixel_raw, 0, 0};
                bgfifo.push(pxl);
            }
            for(int i = 0; i < lineobjs; i++){
                if(((fetchedobjs[i].x-8) > fetchX && (fetchedobjs[i].x-8) < fetchX+8) || 
                    ((fetchedobjs[i].x) > fetchX && (fetchedobjs[i].x) < fetchX+8)) {
                        state = SPRITE; // We need to parse a spriet
                        spstate = ADVANCE;
                        curr_sprite = fetchedobjs[i];
                        break;
                    }
            }
            if(state != SPRITE){
                pixel_t temp = {0x0, 0x0, 0};
                for(int i = fgfifo.size(); i < 8; i++){
                    fgfifo.push(temp);
                }
                state = GET_TILE;
                fetchX += 8;
                is_render_ready = true;
                if(fetchX > WIDTH){
                    mode = M1;
                }
            }
        }
    } else if(state == SPRITE){
        if(spstate == ADVANCE){
            if(!isAdvance){
                isAdvance = true;
            } else {
                spstate = RETRIEVE;
                isAdvance = false;
            }
        } else if(spstate == RETRIEVE){
            uint8_t row = ((curr_sprite.y - 16) - (*LY)) & 7;
            uint16_t tiledata = 0x8000 + curr_sprite.idx*16 + row*2;
            tilelow = mem->get(tiledata);
            tilehigh = mem->get(tiledata+1);
            spstate = PUSH_SPRITE;        
        } else {
            pixel_t temp = {0x0, 0x0, 0};
            if(fgfifo.size() < 8){
                for(int i = fgfifo.size(); i < 8; i++){
                    fgfifo.push(temp);
                }
            }
            uint16_t fulltile = tilelow + (tilehigh*0x100);
            std::queue<pixel_t> new_fgfifo;            
            for(int i = 0; i < std::max(0, curr_sprite.x - fetchX); i++){
                new_fgfifo.push(temp);
            }
            for(int i = std::max(0, curr_sprite.x - fetchX); i < std::min(fetchX - curr_sprite.x, 8); i++){
                uint8_t pixel_raw = fulltile & (0b11 << (i*2));                
                pixel_t pxl = {pixel_raw, (uint8_t)(curr_sprite.flag&(1<<4)), (uint8_t)(curr_sprite.flag&(1<<7))};
                new_fgfifo.push(pxl);
            }
            std::queue<pixel_t> combined_fifo;
            for(int i = 0; i < std::min(fetchX - curr_sprite.x, 8); i++){
                pixel_t old_pxl = fgfifo.front();
                fgfifo.pop();
                pixel_t new_pxl = new_fgfifo.front();
                new_fgfifo.pop();
                pixel_t pxl = (old_pxl.color == 0)?new_pxl:old_pxl;
                combined_fifo.push(pxl);
            }
            std::swap(fgfifo, combined_fifo);
            for(int i = fgfifo.size(); i < 8; i++){
                fgfifo.push(temp);
            }
            // remove object
            fetchedobjs.erase(std::remove(fetchedobjs.begin(), fetchedobjs.end(), curr_sprite), fetchedobjs.end());
            // see if there is another sprite
            for(int i = 0; i < lineobjs; i++){
                if(((fetchedobjs[i].x-8) > fetchX && (fetchedobjs[i].x-8) < fetchX+8) || 
                    ((fetchedobjs[i].x) > fetchX && (fetchedobjs[i].x) < fetchX+8)) {
                        spstate = ADVANCE;
                        curr_sprite = fetchedobjs[i];
                        break;
                }
            }
            if(spstate != ADVANCE){
                state = GET_TILE;
                fetchX += 8;
                is_render_ready = true;
                if(fetchX > WIDTH){
                    mode = M1;
                }
            }
        }
    }
}

void PPU::updt_drawpxl(){
    pxl_fetcher();
    // display what is in the queue
    if(is_render_ready){
        if(displayX > 0 && displayX < 8){
            for(int i = 0; i < displayX; i++){
                lcd[i][(*LY)] = 0;
            }
        }
        for(int i = 0; i < 8; i++){
            pixel_t bg_pxl = bgfifo.front();
            bgfifo.pop();
            pixel_t fg_pxl = fgfifo.front();
            fgfifo.pop();
            pixel_t pxl = (fg_pxl.bgpriority > 0) ? fg_pxl : bg_pxl;
            if(pxl.color == 0){
                pxl.color = fg_pxl.color + bg_pxl.color; // This is stupid
            }
            lcd[displayX+i][(*LY)] = pxl.color;
        }
        displayX += 8;
        is_render_ready = false;
    }
}

bool PPU::getlcdc(int val){
    int temp = (int)(pow(2, val));
    if ((temp & (*LCDC)) > 0){
        return true;
    }
    return false;
}
