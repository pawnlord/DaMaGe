#include "gbdisplay.h"
#include "gbprocessor.h"
#include <time.h>
int main(int argc, char** argv){
    std::string cartridge_name;
    std::string config_name = "";
    if(argc == 1){
        std::cout << "[DaMaGe] Input name of ROM you would like to play (no spaces): " << std::endl;
        std::string game;
        std::cin >> game;
        cartridge_name = game;
    } else {
        for(int i = 1; i < argc; i++){
            if(std::string(argv[i]) == "-v" || std::string(argv[i]) == "-V"){
                std::cout << "DaMaGe - Original Gameboy Emulator" << std::endl;
                std::cout << "Version 1.0.0" << std::endl;
                return 0;
            } else if(std::string(argv[i]) == "-c" || std::string(argv[i]) == "--config"){
                if(++i < argc){
                    config_name = argv[i];
                } else {
                    std::cerr << "[DaMaGe] Expected custom config, but none provided.\n";
                }
            } else {
                cartridge_name = argv[i];    
            }
        }
    }

    GameboyDisplay gd;


    EmulatorConfig cfg;
    
    if(config_name != ""){
        cfg = EmulatorConfig(config_name);
    }

    Memory mem(gd.gm.input, cfg);
    
    mem.load_cartridge(cartridge_name);

    Clock clock(&mem);
    CPU cpu(&mem, &clock);

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