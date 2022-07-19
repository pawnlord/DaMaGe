#include "config.h"
#include <SDL2/SDL.h>

EmulatorConfig::EmulatorConfig(){
    ops_mult = 2;
    layout[0] = {"A", SDL_SCANCODE_A};
    layout[1] = {"B", SDL_SCANCODE_S};
    layout[2] = {"SLCT", SDL_SCANCODE_Q};
    layout[3] = {"STRT", SDL_SCANCODE_W};
    layout[4] = {"U", SDL_SCANCODE_UP};
    layout[5] = {"D", SDL_SCANCODE_DOWN};
    layout[6] = {"L", SDL_SCANCODE_LEFT};
    layout[7] = {"R", SDL_SCANCODE_RIGHT};
}

SDL_Scancode string_to_scancode(std::string keyname){
    return SDL_GetScancodeFromName(keyname.c_str());
}

EmulatorConfig::EmulatorConfig(std::string filename){
    // Defaults
    ops_mult = 2;
    layout[0] = {"A", SDL_SCANCODE_A};
    layout[1] = {"B", SDL_SCANCODE_S};
    layout[2] = {"SLCT", SDL_SCANCODE_Q};
    layout[3] = {"STRT", SDL_SCANCODE_W};
    layout[4] = {"U", SDL_SCANCODE_UP};
    layout[5] = {"D", SDL_SCANCODE_DOWN};
    layout[6] = {"L", SDL_SCANCODE_LEFT};
    layout[7] = {"R", SDL_SCANCODE_RIGHT};
    layout[8] = {"SPEED_CHANGE", SDL_SCANCODE_LCTRL};

    
    std::ifstream ifs (filename, std::ifstream::binary);
    std::string line;
    while(std::getline(ifs, line)){
        std::string token;
        std::stringstream linestream(line);
        if(std::getline(linestream, token, ' ')){
            for(int i = 0; i < 9; i++){
                if(token != layout[i].name){
                    continue;
                }

                if(!std::getline(linestream, token, '\r')){
                    break;
                }
                
                SDL_Scancode key = string_to_scancode(token);
                if(key != SDL_SCANCODE_UNKNOWN){
                    layout[i].key = key;
                }
                break;
            }
            if(token == "SPEED"){
                if(std::getline(linestream, token, '\r')){
                    ops_mult = std::atof(token.c_str());
                }
            }
        
        }
    }
}
    