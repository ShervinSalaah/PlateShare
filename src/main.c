/**
 * @file main.c
 * @brief Entry point for PlateShare Pro
 * @author Member 1 (Integration)
 */

#include "config.h"
#include "core.h"
#include "user.h"
#include "plate.h"
#include "request.h"
#include "chat.h"

/* ========== Global Database ========== */
Config appConfig;
User users[MAX_USERS];       int userCount = 0;
Plate plates[MAX_PLATES];    int plateCount = 0;
Request requests[MAX_REQUESTS]; int requestCount = 0;

int main(void) {
    initSystem(&appConfig);
    loadUsers(users, &userCount);
    loadPlates(plates, &plateCount);
    loadRequests(requests, &requestCount);
    runMenuSystem();
    return 0;
}