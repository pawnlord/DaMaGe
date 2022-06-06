#include "graphics.h"
#include <time.h>

int main()
{   
    GraphicsManager gm("testing", 160, 144, 3, 20);
    time_t t = time(NULL);
    time_t difference = 0;
    gm.start();
    while(difference < 10){
        difference = time(NULL) - t;
    }
    gm.stopSDL();
    return EXIT_SUCCESS;
}