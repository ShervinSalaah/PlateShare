#ifndef CORE_H
#define CORE_H

#include "config.h"

void initSystem(Config *cfg);
void getCurrentTimestamp(char *buffer, size_t size);
void loadConfig(Config *cfg);
void saveConfig(const Config *cfg);
int validateFilePath(const char *path);
void runMenuSystem(void);
void printCentered(const char *str);
void printCenteredLine(char ch, int width);

#endif