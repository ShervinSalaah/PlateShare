/**
 * @file core.h
 * @brief Core system interface: menu, configuration, utilities
 * @author Member 1
 * 
 * Provides system initialization, menu display functions,
 * configuration management, and shared utility functions
 * used across all modules.
 */

#ifndef CORE_H
#define CORE_H

#include "config.h"

/* ========== System Initialization ========== */
void initSystem(Config *cfg);

/* ========== Timestamp Utilities ========== */
void getCurrentTimestamp(char *buffer, size_t size);

/**
 * @brief Gets today's date in YYYY-MM-DD format for date comparison
 * @param buffer Buffer to store the date string (must be >= 11 chars)
 */
void getCurrentDate(char *buffer);

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
 * @brief Pauses the screen so the user can read messages
 * @param message Text to display before waiting
 * 
 * Prints the message and waits for the user to press Enter.
 * Used after every operation to prevent messages from disappearing.
 */
void pauseScreen(const char *message);

#endif