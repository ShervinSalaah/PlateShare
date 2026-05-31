#ifndef CORE_H 
#define CORE_H 
#include "config.h" 
 
void initSystem(Config *cfg); 
int displayMainMenu(); 
void loadConfig(Config *cfg); 
void saveConfig(const Config *cfg); 
int validateFilePath(const char *path); 
 
#endif 
