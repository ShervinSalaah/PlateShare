/**
 * @file main.c
 * @brief Entry point for PlateShare Pro
 * @author Member 1 (Integration)
 * 
 * This file defines the global database (all arrays) and starts
 * the application. It initializes the system, loads all persistent
 * data from CSV files, and launches the menu loop.
 */

#include "config.h"
#include "core.h"
#include "user.h"
#include "plate.h"
#include "request.h"
#include "chat.h"

/* ========== Global Database ========== */
Config appConfig;
User users[MAX_USERS];            int userCount = 0;
Plate plates[MAX_PLATES];         int plateCount = 0;
Request requests[MAX_REQUESTS];   int requestCount = 0;

/**
 * @brief Program starts here
 * 
 * 1. Initializes the system (folders, config)
 * 2. Loads all data from CSV files
 * 3. Starts the menu loop
 * 4. Exits cleanly when the user chooses Exit
 */
int main(void) {
    initSystem(&appConfig);
    loadUsers(users, &userCount);
    loadPlates(plates, &plateCount);
    loadRequests(requests, &requestCount);
    runMenuSystem();
    return 0;
}