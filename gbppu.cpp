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
    this->lcd = (layer)malloc(WIDTH*sizeof(uint8_t*));
    for(int i = 0; i < WIDTH; i++){
        lcd[i] = (uint8_t*)malloc(HEIGHT);
        memset(lcd[i], 0, HEIGHT);
    } 
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
        
        if(((*STAT) & 0x1<<(3+mode)) > 0 && mode != M3){ // switch mode 
            if(mem->get_int_enabled(0x2))
                mem->req_int(0x2);
        }
        if((*LY) == 143){
            mem->req_int(0x1);
        }
        
        // Interrupts
        if((*LY) == (*LYC)){
            (*STAT) |= 1<<0x2; // set LYC=LY
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

    if(ppu_enabled()){
        (*STAT) = ((*STAT) & ~(0b11)) | mode;
        if(mode == M2){
            updt_oamscan();
        } else if (mode == M3){
            updt_drawpxl();
        }
    }
}


void PPU::init_drawpxl(){
    static std::queue<pixel_t> empty;
    std::swap(bgfifo, empty);
    fgfifo.clear();

    mode = M3;

    uint8_t offset = ((*SCX)%8);
    fetchX = -offset;
    displayX = -offset;
    fetchWidth = WIDTH;//+ ((8-offset)%8);
}

void PPU::updt_oamscan(){
    static int objcount = 0;
    object_t obj = OAM[objcount];

    bool is_in_line = ((*LY) >= obj.y-16) && ((*LY) < (obj.y-16+(getlcdc(2)?16:8)));
    if(is_in_line && lineobjs < 10 && totalobjs < 40){
        lineobjs++;
        if((*LY) == obj.y-16){
            totalobjs++;
        }
        fetchedobjs.push_back(obj);
    }

    objcount++;

    if(objcount == 40){
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
        isWin = (*LY) >= (*WY) && fetchX >= (*WX)-7 && getlcdc(5);
        uint16_t tilemap, fetcherX, fetcherY;
        tilemap = 0x9800;

        if((isWin && getlcdc(6)) || (!isWin && getlcdc(3))){
            tilemap = 0x9C00;
        }

        if(!isWin){
            fetcherX = (((*SCX)+fetchX )/8)%32;
            fetcherY = (((*LY)+(*SCY))/8)%32;
        } else {
            fetcherX = (fetchX - ((*WX) - 7)) / 8;
            fetcherY = ((*LY) - (*WY)) / 8;    
        }
        tile = mem->get(tilemap + (fetcherX + fetcherY*32));
        state = (state_e)(((int)state) + 1);
    } else if(state == GET_LOW){
        uint8_t row = (isWin)? ((*LY)-(*WY))&7: ((*LY)+(*SCY))&7;
        uint16_t tiledata = getlcdc(4)? 0x8000 + tile*16 + row*2: 0x9000 + ((int8_t)tile)*16 + row*2;

        tilelow = mem->get(tiledata);
        state = (state_e)(((int)state) + 1);
    } else if(state == GET_HIGH){
        uint8_t row = (isWin)? ((*LY)-(*WY))&7: ((*LY)+(*SCY))&7;
        uint16_t tiledata = getlcdc(4)? 0x8000 + tile*16 + row*2 + 1 : 0x9000 + ((int8_t)tile)*16 + row*2 + 1;

        tilehigh = mem->get(tiledata);
        state = (state_e)(((int)state) + 1);
    } else if(state == SLEEP){
        state = (state_e)(((int)state) + 1);
    } else if(state == PUSH){
        if(bgfifo.size() < 16){
            uint16_t fulltile = tilelow + (tilehigh*0x100);

            for(int i = 7; i >= 0; i--){
                uint8_t col = get_color(fulltile, i);
                pixel_t pxl = {col, 0, 0};
                bgfifo.push(pxl);
            }

            currobjs.clear();
            for(int i = 0; i < lineobjs; i++){
                if((fetchX >= fetchedobjs[i].x-8 && fetchX <= fetchedobjs[i].x) || 
                   (fetchedobjs[i].x-8 >= fetchX && fetchedobjs[i].x-8 <= fetchX+8)) {
                    state = SPRITE; // We need to parse a sprite
                    spstate = ADVANCE;
                    currobjs.push_back(fetchedobjs[i]);
                    curr_sprite = currobjs[0];
                    currobjidx = 0;
                }
            }

            if(state != SPRITE){
                pixel_t temp = {0x0, 0x0, 0};

                for(int i = fgfifo.size(); i < 8; i++){
                    fgfifo.push_back(temp);
                }

                state = GET_TILE;
                fetchX += 8;
                is_render_ready = true;

                if(fetchX >= fetchWidth){
                    mode = M0;
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
            uint8_t row = ((*LY) - (curr_sprite.y - 16)) & 7;
            uint16_t tiledata = 0x8000 + curr_sprite.idx*16 + row*2;

            tilelow = mem->get(tiledata);
            tilehigh = mem->get(tiledata+1);

            spstate = PUSH_SPRITE;        
        } else {
            pixel_t temp = {0x0, 0x0, 0};

            for(int i = fgfifo.size(); i < 8; i++){
                fgfifo.push_back(temp);
            }

            uint16_t fulltile = tilelow + (tilehigh*0x100);
            std::vector<pixel_t> new_fgfifo;            
            bool reverse = curr_sprite.flag&(1<<5);
            int16_t firstpxloff;
            
            int16_t start = std::max(curr_sprite.x - 8 - fetchX, 0);
            int16_t end   = std::min(curr_sprite.x-fetchX, 8); // positional, doesn't depend on 
            if(reverse){
                firstpxloff = (start==0)?8-end:-start; // depends on which side we are on
            } else { 
                firstpxloff = (start==0)?-start:8-end;
            }

            for(int i = start; i < end; i++){
                uint8_t col = get_color(fulltile, i+firstpxloff);
                pixel_t pxl = {col, (uint8_t)(curr_sprite.flag&(1<<4)), (uint8_t)(curr_sprite.flag&(1<<7))};
                if(reverse){
                    new_fgfifo.push_back(pxl);
                } else {
                    new_fgfifo.insert(new_fgfifo.begin(), pxl);
                }
            }

            for(int i = 0; i < start; i++){
                new_fgfifo.insert(new_fgfifo.begin(), temp);
            }
            for(int i = end; i < 8; i++){
                new_fgfifo.push_back(temp);
            }


            for(int i = 0; i < 8; i++){
                pixel_t old_pxl = fgfifo[i];
                pixel_t new_pxl = new_fgfifo[i];

                pixel_t pxl = (old_pxl.color == 0)?new_pxl:old_pxl;
                fgfifo[i] = pxl;
            }
 
            currobjidx+=1;
            if(currobjidx < currobjs.size()){
                curr_sprite = currobjs[currobjidx];
                spstate = ADVANCE;
            }

            if(spstate != ADVANCE){
                state = GET_TILE;
                fetchX += 8;
                is_render_ready = true;
                if(fetchX >= fetchWidth){
                    mode = M0;
                }
            }
        }
    }
}

uint8_t PPU::get_color(uint16_t fulltile, uint8_t i){
    uint16_t pixel_raw = (fulltile & (0x101 << (i))) >> (i);                
    uint8_t col = 0;

    if((pixel_raw&0x100) > 0){
        col += 0b10;
    } 

    if((pixel_raw&0x1) > 0){
        col += 0b1;
    }
    return col;
}


void PPU::updt_drawpxl(){
    pxl_fetcher();
    // display what is in the queue
    if(is_render_ready){

        for(int i = 0; i < 8; i++){
            pixel_t bg_pxl = bgfifo.front();
            bgfifo.pop();
            pixel_t fg_pxl = fgfifo[i];
            pixel_t pxl = (fg_pxl.bgpriority > 0) ? bg_pxl : fg_pxl;
            if(pxl.color == 0){
                pxl.color = fg_pxl.color + bg_pxl.color; // This is stupid
            }
            if(displayX+i >= 0 && displayX+i < WIDTH){
                lcd[displayX+i][(*LY)] = pxl.color;
            }
        }
        fgfifo.clear();
        displayX += 8;
        is_render_ready = false;
    }
}

bool PPU::getlcdc(int val){
    return (*LCDC) & (1<<val);
}

bool PPU::ppu_enabled(){
    return getlcdc(7);
}

uint8_t PPU::getstatfull(){
    return (*STAT);
}
uint8_t PPU::getlcdcfull(){
    return (*LCDC);
}
void PPU::add_debug_vector(std::vector<std::string> *dbg){
    this->dbg = dbg;
    has_dbg_vec = true;
}
