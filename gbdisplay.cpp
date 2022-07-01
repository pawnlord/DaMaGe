#include "gbdisplay.h"

GameboyDisplay::GameboyDisplay(PPU* ppu)
    : gm("Gameboy", WIDTH, HEIGHT, PIXEL_SIZE, HEADER) {
    this->ppu = ppu;
    gm.add_loop(this);
}

void GameboyDisplay::update_gm_pixels(){
    static int test = 1;
    int col;
    for(int i = 0; i < WIDTH; i++){
        for(int j = 0; j < HEIGHT; j++){
            col = 255 * ((3.0-(float)(ppu->lcd[i][j]))/3);                
            if(test == j) {col = 0;}
            gm.set_pxl(i, j, col);
        }
    }
    test += 1;
    test %= HEIGHT;
}
