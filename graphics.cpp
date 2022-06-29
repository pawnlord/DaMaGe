#include "graphics.h"

Pixel::Pixel(int x, int y, int s, int r, int g, int b){
    this->x = x;
    this->y = y;
    this->w = s;
    this->h = s;

    this->r = r;
    this->g = g;
    this->b = b;
}

void Pixel::set_color(int r, int g, int b){
    this->r = r;
    this->g = g;
    this->b = b;
}

void Pixel::set_color(int v){
    this->r = v;
    this->g = v;
    this->b = v;
}

void GraphicsManager::stopSDL(){
    // TODO: Add the rest of the objects created 
    runmx.lock();
    if (ren != nullptr) {
        SDL_DestroyRenderer(ren);
    }
    if (win != nullptr) {
        SDL_DestroyWindow(win);
        SDL_Quit();
    }
    running = false;
    runmx.unlock();
    if(graphicsThread.joinable()){
        graphicsThread.join();
    }
}

void GraphicsManager::sdl_check_loud(void* value){
    if(!value){
        std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
        exit(1);   
        stopSDL();
    }
}
void GraphicsManager::sdl_check_loud(void* value, void* against){
    if(value == against){
        std::cerr << "SDL Error: " << SDL_GetError() << std::endl;
        exit(1);   
        stopSDL();  
    }
}

void GraphicsManager::add_sprite(std::vector<std::vector<int> > pixels, int x, int y){
    for(int i = 0; i < pixels.size(); i++){
        for(int j = 0; j < pixels[0].size(); j++){
            this->pixs[x+i][y+j].set_color(pixels[i][j]);
        }
    }
}
void GraphicsManager::set_pxl(int x, int y, int col){
    if(x < pixs.size() && y < pixs[0].size()){
        this->pixs[x][y].set_color(col);
    }
}
void GraphicsManager::set_pxl(int x, int y, int r, int g, int b){
    this->pixs[x][y].set_color(r, g, b);
}


void GraphicsManager::render_cb(){
    SDL_Event evt;
    while(running){
        this->runmx.lock();    
        while( SDL_PollEvent(&evt) ) {
            switch(evt.type) {
                case SDL_QUIT:
                    std::cout << "QUIT\n";
                    running = false;   
                break;
            }
        }


        // update pixels
        this->loop->update_gm_pixels();
        // Pixel renderer
        SDL_RenderClear(ren);
        for(int i = 0; i < w; i++){
            for(int j = 0; j < h; j++){
                SDL_SetRenderDrawColor(ren, pixs[i][j].r, pixs[i][j].g, pixs[i][j].b, 255);
                SDL_RenderFillRect(ren, &pixs[i][j]);
            }
        }
        SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
        SDL_RenderPresent(ren);
        SDL_Delay(100);
        this->runmx.unlock(); 
    }
}

GraphicsManager::GraphicsManager(std::string name, int w, int h, int s, int head){
    win = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w*s, h*s+head, SDL_WINDOW_SHOWN);
    sdl_check_loud((void*) win);    
    ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    sdl_check_loud((void*) ren);
    this->w = w;
    this->h = h;
    for(int i = 0; i < w; i++){
        pixs.push_back(std::vector<Pixel>());
        for(int j = 0; j < h; j++){
            pixs[i].push_back(Pixel(s*i,s*j+head,s,255 * (i%2),255* (i%2),255* (i%2)));
        }
    }
}

void GraphicsManager::start(){
    graphicsThread = std::thread(&GraphicsManager::render_cb, this);
}


void GraphicsManager::add_loop(MainLoop* mainloop){
    this->loop = mainloop;
}
