# DaMaGe
Original Gameboy (DMG) emulator written in C++  
Version 1.2.0

## building/dependencies
Currently, this project requires SDL2, SDL2_ttf, and (on windows) a MinGW thread library.  
```
g++ *.cpp -lmingw32 -lSDL2 -lSDL2_ttf -lSDL2main -std=c++11 -o DaMaGe.exe
```

## usage
The usage is currently pretty basic, I plan to add some sort of interface when more advanced features are added.  
```
DaMaGe <ROM_name> [-c/--config <config_name>] [--load <game_name>]
```
Other options:  
```
-v/--version: print version information of DaMaGe
```

### keybindings
As for using the emulator part, the controls for now are as follows:  
|  A  |  B  |select|start|   D-Pad  | Change Speed* | Save Savestate | Load Savestate |
|-----|-----|------|-----|----------|---------------|----------------|----------------| 
|  A  |  S  |  Q   |  W  |Arrow Keys|  L-Control    |   K            |     L          |

<sup>* Double Speed by default</sup>

### save states
Because actual saves have not been implemented yet, you need to use save states.  
You can directly load a savestate using the `--load` option when booting up. Because of how it works udner the hood, you supply the game name and not a file name (So, if you want to load "savestate1.sav", you would use `DaMaGe --load savestate1`). There is also currently no way to save multiple save states and choose which one to load in-game. The save state used by a game loaded from ROM will be `<game_name>.sav`.  

## config
The config style is pretty simple, where each line changes an option to a value like this:  
```
<option> <value>
```
Currently implemented options:  
```
U
D
L
R
A
B
SELECT
START
SAVESTATE_SV
SAVESTATE_LD
CHANGE_SPEED
SPEED_MULT
```
The first 11 correspond to the table above, while the last changes the speed multiplier when pressing Change Speed.  
The values for the key bind options come from [the SDL library](https://wiki.libsdl.org/SDL_Scancode). The value for SPEED_MULT is a floating-point number.
## TODO
 - Serial input over network (Biggest goal)
 - GUI implemented
    - Possibly CLI style?
        - Not intuitive to use, but intuitive to make, and allows the possibility to keep debuging tools that already have been made.
 - Sound
    - Great extra project, but doesn't seem as interesting as the rest
 - Saves if I feel like it.
