#include "gbdisplay.h"
#include "gbprocessor.h"
#include <time.h>
int main(int argc, char** argv){
    std::string cartridge_name = "";
    std::string config_name = "", savestate = "";
    bool isSaveState = false;
    for(int i = 1; i < argc; i++){
        if(std::string(argv[i]) == "-v" || std::string(argv[i]) == "-V"){
            std::cout << "DaMaGe - Original Gameboy Emulator" << std::endl;
            std::cout << "Version 1.1.0" << std::endl;
            return 0;
        } else if(std::string(argv[i]) == "-c" || std::string(argv[i]) == "--config"){
            if(++i < argc){
                config_name = argv[i];
            } else {
                std::cerr << "[DaMaGe] Warning: Expected custom config, but none provided.\n";
            }
        } else if(std::string(argv[i]) == "--load"){
            if(++i < argc){
                savestate = argv[i];
            } else {
                std::cout << "[DaMaGe] No Save State provided, automating to game title." << std::endl;
            }
            isSaveState = true;
        } else {
            cartridge_name = argv[i];    
        }
    }
    
    if(cartridge_name == "" && savestate == ""){
        std::cout << "[DaMaGe] Input name of ROM you would like to play (no spaces): " << std::endl;
        std::string game;
        std::cin >> game;
        cartridge_name = game;
    }

    GameboyDisplay gd;

    EmulatorConfig cfg;
    
    if(config_name != ""){
        cfg = EmulatorConfig(config_name);
    }

    Memory mem(gd.gm.input, cfg);
    if(!isSaveState){
        mem.load_cartridge(cartridge_name);
    }
    Clock clock(&mem);
    CPU cpu(&mem, &clock);
    if(isSaveState){
        if(savestate == ""){ savestate = mem.rom_name; }
        cpu.load_savestate(savestate);
    }
#ifdef DEBUG_TOOLBAR
    std::vector<std::string> data;
    gd.gm.add_data_ptr(&data);
    cpu.getppu()->add_debug_vector(&data);
#endif
    cpu.set_change_speed(cfg.ops_mult);

    gd.add_ppu(cpu.getppu());
    gd.gm.start();
    cpu.run();
    
    gd.gm.stopSDL();
    return EXIT_SUCCESS;
}