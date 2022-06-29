#include "gbdisplay.h"

GameboyDisplay::GameboyDisplay(PPU* ppu)
    : gm("Gameboy", WIDTH, HEIGHT, PIXEL_SIZE, HEADER) {
    this->ppu = ppu;
    gm.add_loop(this);
}

void GameboyDisplay::update_gm_pixels(){
    if(memcmp(last_lcd, ppu->lcd, sizeof(last_lcd))){
        int col;
        for(int i = 0; i < WIDTH; i++){
            for(int j = 0; j < HEIGHT; j++){
                col = 255 * ((3.0-(float)(ppu->lcd[i][j]))/3);                
                gm.set_pxl(i, j, col);
            }
        }
        memcpy(&last_lcd, &(ppu->lcd), sizeof(last_lcd));
    }
}
