/**
 * @file core.h
 * @brief Core system interface: menu, configuration, utilities
 * @author Member 1
 */

#ifndef CORE_H
#define CORE_H

#include "config.h"

/* ========== System Initialization ========== */
void initSystem(Config *cfg);

/* ========== Timestamp Utility ========== */
void getCurrentTimestamp(char *buffer, size_t size);

/* ========== Configuration Management ========== */
void loadConfig(Config *cfg);
void saveConfig(const Config *cfg);
int validateFilePath(const char *path);

/* ========== Menu System ========== */
void runMenuSystem(void);

/* ========== Display Utilities ========== */
void printCentered(const char *str);
void printCenteredLine(char ch, int width);

/**
 * @brief Gets the current date in YYYY-MM-DD format for date comparison
 * @param buffer Buffer to store the date string
 */
void getCurrentDate(char *buffer);

#endif