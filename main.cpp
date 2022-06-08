#include "graphics.h"
#include <time.h>

int main()
{   
    GraphicsManager gm("testing", 160, 144, 3, 20);
    time_t t = time(NULL);
    time_t difference = 0;
    gm.start();
    std::vector<std::vector<int> > sprite_val = {
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 255, 0, 255, 0, 0},
        {0, 255, 0, 0, 0, 0, 0, 255},
        {0, 255, 0, 0, 0, 0, 0, 255},
        {0, 0, 255, 0, 0, 0, 255, 0},
        {0, 0, 0, 255, 255, 255, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0},
    };
    while(difference < 10){
        
        SDL_Event event;
        if(SDL_PollEvent(&event)){
            if (event.type == SDL_QUIT){
                break;
            }
        }
        if(difference > 5){
            gm.add_sprite(sprite_val, 1, 1);
        }
        difference = time(NULL) - t;  
    }
    gm.stopSDL();
    return EXIT_SUCCESS;
}