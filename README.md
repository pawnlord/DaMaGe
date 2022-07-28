# DaMaGe
Original Gameboy (DMG) emulator written in C++  
Version 1.2.0

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
|  A  |  B  |select|start|   D-Pad  | Change Speed* |
|-----|-----|------|-----|----------|---------------|
|  A  |  S  |  Q   |  W  |Arrow Keys|  L-Control    |
<sup>* Double Speed by default</sup>

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
CHANGE_SPEED
SPEED_MULT
```
The first 9 correspond to the table above, while the last changes the speed multiplier when pressing Change Speed.  
The values for the key bind options come from [the SDL library](https://wiki.libsdl.org/SDL_Scancode). The value for SPEED_MULT is a floating-point number.
## TODO
 - Serial input over network (Biggest goal)
 - GUI implemented
    - Possibly CLI style?
        - Not intuitive to use, but intuitive to make, and allows the possibility to keep debuging tools that already have been made.
 - Sound
    - Great extra project, but doesn't seem as interesting as the rest
 - Saves if I feel like it.
