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
    if (ren != nullptr) {
        SDL_DestroyRenderer(ren);
    }
    if (win != nullptr) {
        SDL_DestroyWindow(win);
        SDL_Quit();
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

void GraphicsManager::render_cb(){

}

GraphicsManager::GraphicsManager(std::string name, int w, int h, int s, int head){
    win = SDL_CreateWindow(name.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, w*s, h*s+head, SDL_WINDOW_SHOWN);
    sdl_check_loud((void*) win);    
    SDL_Renderer* ren = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    sdl_check_loud((void*) ren);

    for(int i = 0; i < h; i++){
        pixs.push_back(std::vector<Pixel>());
        for(int j = 0; j < w; j++){
            pixs[i].push_back(Pixel(s*j,s*i,s,0,0,0));
        }
    }

}

void GraphicsManager::start(){
    graphicsThread = std::thread(&GraphicsManager::render_cb, this);
}

int main()
{
    using std::cerr;
    using std::endl;

    SDL_Window* win = SDL_CreateWindow("Hello World!", 100, 100, 620, 387, SDL_WINDOW_SHOWN);
    if (win == nullptr) {
        cerr << "SDL_CreateWindow Error: " << SDL_GetError() << endl;
        return EXIT_FAILURE;
    }
    SDL_Renderer* ren
        = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (ren == nullptr) {
        cerr << "SDL_CreateRenderer Error" << SDL_GetError() << endl;
		if (win != nullptr) {
			SDL_DestroyWindow(win);
		}
		SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Surface* bmp = SDL_LoadBMP("img.bmp");
    if (bmp == nullptr) {
        cerr << "SDL_LoadBMP Error: " << SDL_GetError() << endl;
		if (ren != nullptr) {
			SDL_DestroyRenderer(ren);
		}
		if (win != nullptr) {
			SDL_DestroyWindow(win);
		}
		SDL_Quit();
        return EXIT_FAILURE;
    }

    SDL_Texture* tex = SDL_CreateTextureFromSurface(ren, bmp);
    if (tex == nullptr) {
        cerr << "SDL_CreateTextureFromSurface Error: " << SDL_GetError() << endl;
		if (bmp != nullptr) {
			SDL_FreeSurface(bmp);
		}
		if (ren != nullptr) {
			SDL_DestroyRenderer(ren);
		}
		if (win != nullptr) {
			SDL_DestroyWindow(win);
		}
		SDL_Quit();
        return EXIT_FAILURE;
    }
    SDL_FreeSurface(bmp);

    for (int i = 0; i < 20; i++) {
        SDL_RenderClear(ren);
        SDL_RenderCopy(ren, tex, nullptr, nullptr);
        SDL_RenderPresent(ren);
        SDL_Delay(100);
    }

    SDL_DestroyTexture(tex);
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(win);
    SDL_Quit();

    return EXIT_SUCCESS;
}