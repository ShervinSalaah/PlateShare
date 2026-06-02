/**
 * @file core.c
 * @brief Core system: menu, dashboard, settings, session management
 * @author Member 1
 * 
 * This file is the control center of the application. It manages:
 * - User session (login/logout state)
 * - Main menu display (different menus for logged-in vs logged-out)
 * - Dashboard with user statistics and notifications
 * - Settings management (data folder configuration)
 * - Screen display utilities (centered text, separators, pauses)
 */

#include "core.h"
#include "user.h"
#include "plate.h"
#include "request.h"
#include "chat.h"

/* ========== Global Session State ========== */
char g_loggedInUser[30] = "";  /**< Username of currently logged-in user */
int g_loggedIn = 0;              /**< 1 if someone is logged in, 0 if not */

/* ========== Display Utilities ========== */

/**
 * @brief Prints a string centered on an 80-character wide terminal
 * 
 * Calculates the left padding needed to center the text
 * and prints it followed by a newline character.
 */
void printCentered(const char *str) {
    int len = strlen(str);
    int padding = (80 - len) / 2;
    if (padding < 0) padding = 0;
    for (int i = 0; i < padding; i++) printf(" ");
    printf("%s\n", str);
}

/**
 * @brief Prints a line of repeated characters centered on screen
 * 
 * Used for creating visual separators like ===== or ----- in menus.
 * Example: printCenteredLine('=', 36) produces a centered line of 36 equal signs.
 */
void printCenteredLine(char ch, int width) {
    int padding = (80 - width) / 2;
    if (padding < 0) padding = 0;
    for (int i = 0; i < padding; i++) printf(" ");
    for (int i = 0; i < width; i++) printf("%c", ch);
    printf("\n");
}

/**
 * @brief Pauses the program and waits for the user to press Enter
 * 
 * Displays a message and waits. This prevents success/error messages
 * from disappearing before the user can read them.
 */
void pauseScreen(const char *message) {
    printf("\n  %s\n", message);
    printf("  Press Enter to continue...");
    getchar();
}

/* ========== System Functions ========== */

/**
 * @brief Gets the current date and time as a formatted string
 * 
 * Uses the C standard library time functions to get the current
 * system time and formats it as "YYYY-MM-DD HH:MM:SS".
 */
void getCurrentTimestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

/**
 * @brief Gets today's date in YYYY-MM-DD format
 * 
 * Used for comparing dates (checking if a food expiry date is in the past).
 */
void getCurrentDate(char *buffer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, 11, "%Y-%m-%d", t);
}

/**
 * @brief Initializes the entire application environment
 * 
 * This runs once at startup. It sets default configuration values,
 * creates the data folder if it doesn't exist, and loads any
 * previously saved settings from config.ini.
 */
void initSystem(Config *cfg) {
    strcpy(cfg->dataFolder, ".\\data\\");
    cfg->maxUsers = MAX_USERS;
    cfg->maxPlates = MAX_PLATES;
    cfg->maxRequests = MAX_REQUESTS;
    MKDIR(".\\data");
    loadConfig(cfg);
}

/**
 * @brief Loads application settings from config.ini
 * 
 * Reads key=value pairs from the configuration file.
 * If the file doesn't exist, creates one with default values.
 * Settings include: data folder path, maximum limits.
 */
void loadConfig(Config *cfg) {
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "%sconfig.ini", cfg->dataFolder);
    FILE *fp = fopen(path, "r");
    if (fp == NULL) { saveConfig(cfg); return; }
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp)) {
        char key[50], value[MAX_PATH];
        if (sscanf(line, "%[^=]=%s", key, value) == 2) {
            if (strcmp(key, "dataFolder") == 0) strcpy(cfg->dataFolder, value);
            else if (strcmp(key, "maxUsers") == 0) cfg->maxUsers = atoi(value);
            else if (strcmp(key, "maxPlates") == 0) cfg->maxPlates = atoi(value);
            else if (strcmp(key, "maxRequests") == 0) cfg->maxRequests = atoi(value);
        }
    }
    fclose(fp);
}

/**
 * @brief Saves current application settings to config.ini
 */
void saveConfig(const Config *cfg) {
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "%sconfig.ini", cfg->dataFolder);
    FILE *fp = fopen(path, "w");
    if (fp == NULL) return;
    fprintf(fp, "dataFolder=%s\n", cfg->dataFolder);
    fprintf(fp, "maxUsers=%d\n", cfg->maxUsers);
    fprintf(fp, "maxPlates=%d\n", cfg->maxPlates);
    fprintf(fp, "maxRequests=%d\n", cfg->maxRequests);
    fclose(fp);
}

/**
 * @brief Checks if a file path is valid and writable
 * 
 * Tests by attempting to create and delete a temporary file.
 * Used for validating LAN shared folder paths.
 */
int validateFilePath(const char *path) {
    char testPath[MAX_PATH];
    snprintf(testPath, MAX_PATH, "%stest.tmp", path);
    FILE *fp = fopen(testPath, "w");
    if (fp) { fclose(fp); remove(testPath); return 1; }
    return 0;
}

/* ========== Notification Counter ========== */

/**
 * @brief Counts how many unread notifications a user has
 * 
 * Reads the user's notification file and counts the lines.
 * Each line represents one notification.
 */
static int countNotifications(const char *username) {
    char notifPath[MAX_PATH];
    snprintf(notifPath, MAX_PATH, "%snotifications_%s.txt", appConfig.dataFolder, username);
    FILE *nf = fopen(notifPath, "r");
    int count = 0;
    if (nf) { char d[MAX_LINE]; while (fgets(d, sizeof(d), nf)) count++; fclose(nf); }
    return count;
}

/* ========== Dashboard ========== */

/**
 * @brief Shows the user's dashboard with statistics and notifications
 * 
 * Displays a personalized overview including:
 * - Recent notifications (up to 4 most recent)
 * - Number of plates the user has donated
 * - Number of successfully donated (completed) plates
 * - Number of requests the user has made
 * - Number of accepted requests
 * - Number of pending requests on the user's plates
 */
static void showDashboard(void) {
    system(CLEAR_SCREEN);

    int notifCount = countNotifications(g_loggedInUser);

    /* Calculate user statistics */
    int myPlates = 0, myRequests = 0, pendingRequests = 0;
    int myDonatedPlates = 0, myAcceptedRequests = 0;

    for (int i = 0; i < plateCount; i++) {
        if (strcmp(plates[i].donor, g_loggedInUser) == 0) {
            myPlates++;
            if (strcmp(plates[i].status, "Donated") == 0) myDonatedPlates++;
        }
    }
    for (int i = 0; i < requestCount; i++) {
        if (strcmp(requests[i].requester, g_loggedInUser) == 0) {
            myRequests++;
            if (strcmp(requests[i].status, "Accepted") == 0) myAcceptedRequests++;
        }
    }
    for (int i = 0; i < requestCount; i++) {
        for (int j = 0; j < plateCount; j++) {
            if (plates[j].id == requests[i].plateId &&
                strcmp(plates[j].donor, g_loggedInUser) == 0 &&
                strcmp(requests[i].status, "Pending") == 0) {
                pendingRequests++;
            }
        }
    }

    /* Display dashboard */
    printf("\n");
    printCenteredLine('=', 40);
    printCentered("YOUR HOME PAGE");
    printCenteredLine('=', 40);
    char temp[80];
    snprintf(temp, sizeof(temp), "Welcome, %s!", g_loggedInUser);
    printCentered(temp);
    printCenteredLine('-', 40);

    /* Notifications section */
    if (notifCount > 0) {
        snprintf(temp, sizeof(temp), "ALERTS (%d new):", notifCount);
        printCentered(temp);
        char notifPath[MAX_PATH];
        snprintf(notifPath, MAX_PATH, "%snotifications_%s.txt", appConfig.dataFolder, g_loggedInUser);
        FILE *nf = fopen(notifPath, "r");
        if (nf) {
            char line[MAX_LINE]; int shown = 0;
            while (fgets(line, sizeof(line), nf) && shown < 4) {
                line[strcspn(line, "\n")] = 0;
                printCentered(line); shown++;
            }
            fclose(nf);
            if (notifCount > 4) {
                snprintf(temp, sizeof(temp), "...and %d more alerts", notifCount - 4);
                printCentered(temp);
            }
        }
    } else {
        printCentered("No new alerts.");
    }

    /* Statistics section */
    printCenteredLine('-', 40);
    printCentered("YOUR ACTIVITY:");
    snprintf(temp, sizeof(temp), "Food shared by you:  %d", myPlates); printCentered(temp);
    snprintf(temp, sizeof(temp), "Successfully given:  %d", myDonatedPlates); printCentered(temp);
    snprintf(temp, sizeof(temp), "Food you requested:  %d", myRequests); printCentered(temp);
    snprintf(temp, sizeof(temp), "Requests accepted:   %d", myAcceptedRequests); printCentered(temp);
    snprintf(temp, sizeof(temp), "Waiting for you:     %d", pendingRequests); printCentered(temp);
    printCenteredLine('=', 40);
}

/* ========== Settings ========== */

/**
 * @brief Shows and manages application settings
 * 
 * Allows the user to view current settings and change the
 * data folder path (useful for LAN shared folder setup).
 */
static void showSettings(void) {
    int choice; char input[MAX_LINE]; char temp[80];
    while (1) {
        system(CLEAR_SCREEN);
        printf("\n");
        printCenteredLine('=', 40);
        printCentered("SETTINGS");
        printCenteredLine('=', 40);
        snprintf(temp, sizeof(temp), "Data folder: %s", appConfig.dataFolder); printCentered(temp);
        snprintf(temp, sizeof(temp), "Max users:    %d", appConfig.maxUsers); printCentered(temp);
        snprintf(temp, sizeof(temp), "Max plates:   %d", appConfig.maxPlates); printCentered(temp);
        snprintf(temp, sizeof(temp), "Max requests: %d", appConfig.maxRequests); printCentered(temp);
        printCenteredLine('-', 40);
        printf("                    1. Change data folder\n");
        printf("                    2. Go back\n");
        printCenteredLine('=', 40);
        printf("                    Your choice: ");
        scanf("%d", &choice); getchar();
        if (choice == 1) {
            printf("                    New folder path: ");
            fgets(input, sizeof(input), stdin); input[strcspn(input, "\n")] = 0;
            if (strlen(input) > 0) { strcpy(appConfig.dataFolder, input); saveConfig(&appConfig); printCentered("Updated! Please restart."); }
            pauseScreen("");
        } else if (choice == 2) return;
    }
}

/* ========== Main Menu ========== */

/**
 * @brief Displays the appropriate main menu based on login state
 * 
 * Shows different options depending on whether a user is logged in.
 * When logged in, also shows the notification count below the username.
 */
static void displayMenu(void) {
    system(CLEAR_SCREEN);
    int notifCount = countNotifications(g_loggedInUser);
    printf("\n");
    printCenteredLine('=', 40);
    printCentered("PLATESHARE PRO");
    printCentered("Share Food, Reduce Waste");
    printCenteredLine('=', 40);
    if (g_loggedIn) {
        char temp[80];
        snprintf(temp, sizeof(temp), "Logged in: %s", g_loggedInUser); printCentered(temp);
        if (notifCount > 0) {
            snprintf(temp, sizeof(temp), "Alerts: %d new", notifCount);
            printCentered(temp);
        }
        printCenteredLine('-', 40);
        printf("                    1. Home Page\n");
        printf("                    2. My Account\n");
        printf("                    3. Share Food\n");
        printf("                    4. Request Food\n");
        printf("                    5. Messages & Alerts\n");
        printf("                    6. Settings\n");
        printf("                    7. Sign Out\n");
        printf("                    8. Exit\n");
    } else {
        printf("                    1. Create Account\n");
        printf("                    2. Sign In\n");
        printf("                    3. Exit\n");
    }
    printCenteredLine('=', 40);
    printf("                    Your choice: ");
}

/**
 * @brief Routes the user's menu choice to the correct module
 * 
 * Acts as a traffic controller. Based on the menu number,
 * calls the appropriate function from user, plate, request,
 * or chat modules.
 */
static void handleMenuChoice(int choice) {
    if (!g_loggedIn) {
        switch (choice) {
            case 1: registerUser(users, &userCount); break;
            case 2: g_loggedIn = loginUser(users, userCount, g_loggedInUser); break;
            case 3: printf("\n"); printCentered("Thank you for using PlateShare Pro!"); printf("\n"); exit(0);
            default: printf("\n"); printCentered("Wrong choice! Try again."); pauseScreen("");
        }
    } else {
        switch (choice) {
            case 1:
                loadUsers(users, &userCount); loadPlates(plates, &plateCount); loadRequests(requests, &requestCount);
                showDashboard();
                pauseScreen("");
                break;
            case 2: userMenu(g_loggedInUser, &g_loggedIn); break;
            case 3: plateMenu(g_loggedInUser); break;
            case 4: requestMenu(g_loggedInUser); break;
            case 5: chatMenu(g_loggedInUser); break;
            case 6: showSettings(); break;
            case 7: {
                printf("\n"); printCentered("Are you sure you want to sign out? (y/n): ");
                char confirm; scanf(" %c", &confirm); getchar();
                if (confirm == 'y' || confirm == 'Y') {
                    g_loggedIn = 0; strcpy(g_loggedInUser, "");
                    printCentered("Signed out. See you again!");
                } else { printCentered("Sign out cancelled."); }
                pauseScreen("");
                break;
            }
            case 8: printf("\n"); printCentered("Thank you for using PlateShare Pro!"); printf("\n"); exit(0);
            default: printf("\n"); printCentered("Wrong choice! Try again."); pauseScreen("");
        }
    }
}

/**
 * @brief The main program loop
 * 
 * Runs forever until the user chooses Exit. Displays the menu,
 * reads and validates input, and routes to the correct module.
 */
void runMenuSystem(void) {
    int choice = 0;
    while (1) {
        displayMenu();
        if (scanf("%d", &choice) != 1) {
            printf("\n"); printCentered("Please enter a number!");
            while (getchar() != '\n');
            pauseScreen("");
            continue;
        }
        getchar();
        handleMenuChoice(choice);
    }
}