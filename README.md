# DaMaGe
Original Gameboy (DMG) emulator written in C++  
Version 1.0.0

## building/dependencies
Currently, this project requires SFML and (on windows) a MinGW thread library.  
```
g++ *.cpp -lSDL2 -lSDL2main -std=c++11 -o DaMaGe.exe
```

## usage
The usage is currently pretty basic, I plan to add some sort of interface when more advanced features are added.  
```
DaMaGe <ROM_name>
```
As for using the emulator part, the controls for now are as follows:  
|  A  |  B  |select|start|   D-Pad  |  L-Ctrl      |
|-----|-----|------|-----|----------|--------------|
|  A  |  S  |  Q   |  W  |Arrow Keys| Change Speed*|
<sup>* Currently double speed</sup>

## TODO
 - Fix weird sprite issues
 - Config for controls
 - Serial input over network (Biggest goal)
 - Saves
 - GUI implemented
    - Possibly CLI style?
        - Not intuitive to use, but intuitive to make, and allows the possibility to keep debuging tools that already have been made.
 - Sound
    - Great extra project, but doesn't seem as interesting as the rest