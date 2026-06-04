/**
 * @file core.h
 * @brief Core system interface: menu, configuration, utilities
 * @author 254190K SAMARASINGHE S.M.M.P
 * 
 * Provides system initialization, menu display functions,
 * configuration management, and shared utility functions
 * used across all modules.
 */

#ifndef CORE_H //If the compiler see the same function and declare multiple time, ensure that compiler only read file once.

#define CORE_H //It will define CORE_H Id in memory.

#include "config.h" // It will give address of the original memory location, Therefore, we can use the same memory location to access the data.

/* ========== System Initialization ========== */
void initSystem(Config *cfg); //It wii prepare variables variables. It takes a pointer (*cfg) so it can modify the original configuration data directly in memory.

/* ========== Timestamp Utilities ========== */
void getCurrentTimestamp(char *buffer, size_t size); //Gets the exact current time.
                                                    // size_t size prevents buffer overflows.

/**
 * @brief Gets today's date in YYYY-MM-DD format for date comparison
 * @param buffer Buffer to store the date string (must be >= 11 chars)
 */
void getCurrentDate(char *buffer); //Gets today's date (YYYY-MM-DD).
                                  // Buffer must be at least 11 chars to hold the date and null terminator.

/* ========== Configuration Management ========== */
void loadConfig(Config *cfg); //Reads saved settings from a file into the app.
void saveConfig(const Config *cfg); //Writes current settings to a file.,Uses const. This tells the compiler (and the examiner) that this function will only read the data and promises not to accidentally modify it.
int validateFilePath(const char *path); //Checks if a file path is valid before opening it.,Returns an int (likely 1 for valid, 0 for invalid) to act as a boolean flag.

/* ========== Menu System ========== */
void runMenuSystem(void); //Launches the main text menu loop., Takes void, meaning it requires no inputs to start running.

/* ========== Display Utilities ========== */
void printCentered(const char *str); //Prints a string centered on the console.,
void printCenteredLine(char ch, int width); //Prints a centered line of a specific character (e.g., for separators).

/**
 * @brief Pauses the screen so the user can read messages
 * @param message Text to display before waiting
 * 
 * Prints the message and waits for the user to press Enter.
 * Used after every operation to prevent messages from disappearing.
 */
void pauseScreen(const char *message); //Freezes the screen until you press Enter.

#endif