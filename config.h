#ifndef CONFIG_H
#define CONFIG_H
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <sstream>
#define DEFAULT_SPEED 4194

struct Button{
    std::string name;
    int key;
};

struct EmulatorConfig {
    EmulatorConfig();
    EmulatorConfig(std::string filename);
    Button layout[11];    
    double ops_mult;
};

#endif