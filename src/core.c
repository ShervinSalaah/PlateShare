#include "core.h"
#include "user.h"
#include "plate.h"
#include "request.h"
#include "chat.h"

char g_loggedInUser[30] = "";
int g_loggedIn = 0;

void printCentered(const char *str) {
    int len = strlen(str);
    int padding = (80 - len) / 2;
    if (padding < 0) padding = 0;
    for (int i = 0; i < padding; i++) printf(" ");
    printf("%s\n", str);
}

void printCenteredLine(char ch, int width) {
    int padding = (80 - width) / 2;
    if (padding < 0) padding = 0;
    for (int i = 0; i < padding; i++) printf(" ");
    for (int i = 0; i < width; i++) printf("%c", ch);
    printf("\n");
}

void getCurrentTimestamp(char *buffer, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", t);
}

void initSystem(Config *cfg) {
    strcpy(cfg->dataFolder, ".\\data\\");
    cfg->maxUsers = MAX_USERS;
    cfg->maxPlates = MAX_PLATES;
    cfg->maxRequests = MAX_REQUESTS;
    MKDIR(".\\data");
    loadConfig(cfg);
}

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

int validateFilePath(const char *path) {
    char testPath[MAX_PATH];
    snprintf(testPath, MAX_PATH, "%stest.tmp", path);
    FILE *fp = fopen(testPath, "w");
    if (fp) { fclose(fp); remove(testPath); return 1; }
    return 0;
}

static void showDashboard(void) {
    system(CLEAR_SCREEN);
    char notifPath[MAX_PATH];
    snprintf(notifPath, MAX_PATH, "%snotifications_%s.txt", appConfig.dataFolder, g_loggedInUser);
    FILE *nf = fopen(notifPath, "r");
    int notifCount = 0;
    if (nf) { char d[MAX_LINE]; while (fgets(d, sizeof(d), nf)) notifCount++; fclose(nf); }
    int myPlates = 0, myRequests = 0, pendingRequests = 0;
    for (int i = 0; i < plateCount; i++)
        if (strcmp(plates[i].donor, g_loggedInUser) == 0) myPlates++;
    for (int i = 0; i < requestCount; i++)
        if (strcmp(requests[i].requester, g_loggedInUser) == 0) myRequests++;
    for (int i = 0; i < requestCount; i++)
        for (int j = 0; j < plateCount; j++)
            if (plates[j].id == requests[i].plateId &&
                strcmp(plates[j].donor, g_loggedInUser) == 0 &&
                strcmp(requests[i].status, "Pending") == 0)
                pendingRequests++;
    printf("\n");
    printCenteredLine('=', 36);
    printCentered("DASHBOARD");
    printCenteredLine('=', 36);
    char temp[80];
    snprintf(temp, sizeof(temp), "Welcome, %s", g_loggedInUser);
    printCentered(temp);
    printCenteredLine('-', 36);
    if (notifCount > 0) {
        snprintf(temp, sizeof(temp), "NOTIFICATIONS (%d):", notifCount);
        printCentered(temp);
        nf = fopen(notifPath, "r");
        if (nf) {
            char line[MAX_LINE]; int shown = 0;
            while (fgets(line, sizeof(line), nf) && shown < 4) {
                line[strcspn(line, "\n")] = 0;
                printCentered(line); shown++;
            }
            fclose(nf);
            if (notifCount > 4) {
                snprintf(temp, sizeof(temp), "... and %d more", notifCount - 4);
                printCentered(temp);
            }
        }
    } else {
        printCentered("No new notifications.");
    }
    printCenteredLine('-', 36);
    printCentered("QUICK STATS:");
    snprintf(temp, sizeof(temp), "My Plates:     %d", myPlates); printCentered(temp);
    snprintf(temp, sizeof(temp), "My Requests:   %d", myRequests); printCentered(temp);
    snprintf(temp, sizeof(temp), "Pending Reqs:  %d", pendingRequests); printCentered(temp);
    snprintf(temp, sizeof(temp), "Total Plates:  %d", plateCount); printCentered(temp);
    snprintf(temp, sizeof(temp), "Total Users:   %d", userCount); printCentered(temp);
    printCenteredLine('=', 36);
}

static void showSettings(void) {
    int choice; char input[MAX_LINE]; char temp[80];
    while (1) {
        system(CLEAR_SCREEN);
        printf("\n");
        printCenteredLine('=', 36);
        printCentered("SETTINGS");
        printCenteredLine('=', 36);
        snprintf(temp, sizeof(temp), "Data Folder: %s", appConfig.dataFolder); printCentered(temp);
        snprintf(temp, sizeof(temp), "Max Users:    %d", appConfig.maxUsers); printCentered(temp);
        snprintf(temp, sizeof(temp), "Max Plates:   %d", appConfig.maxPlates); printCentered(temp);
        snprintf(temp, sizeof(temp), "Max Requests: %d", appConfig.maxRequests); printCentered(temp);
        printCenteredLine('-', 36);
        printf("                    1. Change Data Folder\n");
        printf("                    2. Back to Main Menu\n");
        printCenteredLine('=', 36);
        printf("                    Choice: ");
        scanf("%d", &choice); getchar();
        if (choice == 1) {
            printf("                    New data folder path: ");
            fgets(input, sizeof(input), stdin); input[strcspn(input, "\n")] = 0;
            if (strlen(input) > 0) { strcpy(appConfig.dataFolder, input); saveConfig(&appConfig); printCentered("Updated."); }
            printf("                    Press Enter..."); getchar();
        } else if (choice == 2) return;
    }
}

static void displayMenu(void) {
    system(CLEAR_SCREEN);
    char notifPath[MAX_PATH];
    snprintf(notifPath, MAX_PATH, "%snotifications_%s.txt", appConfig.dataFolder, g_loggedInUser);
    FILE *nf = fopen(notifPath, "r");
    int notifCount = 0;
    if (nf) { char d[MAX_LINE]; while (fgets(d, sizeof(d), nf)) notifCount++; fclose(nf); }
    printf("\n");
    printCenteredLine('=', 36);
    printCentered("PLATESHARE PRO");
    printCentered("Community Food Sharing App");
    printCenteredLine('=', 36);
    if (g_loggedIn) {
        char temp[80];
        snprintf(temp, sizeof(temp), "User: %s", g_loggedInUser); printCentered(temp);
        if (notifCount > 0) { snprintf(temp, sizeof(temp), "Notifications: %d", notifCount); printCentered(temp); }
        printCenteredLine('-', 36);
        printf("                    1. Dashboard\n");
        printf("                    2. User Management\n");
        printf("                    3. Plate Management\n");
        printf("                    4. Request Management\n");
        printf("                    5. Chat & Notifications\n");
        printf("                    6. Settings\n");
        printf("                    7. Logout\n");
        printf("                    8. Exit\n");
    } else {
        printf("                    1. Register\n");
        printf("                    2. Login\n");
        printf("                    3. Exit\n");
    }
    printCenteredLine('=', 36);
    printf("                    Choice: ");
}

static void handleMenuChoice(int choice) {
    if (!g_loggedIn) {
        switch (choice) {
            case 1: registerUser(users, &userCount); break;
            case 2: g_loggedIn = loginUser(users, userCount, g_loggedInUser); break;
            case 3: printf("\n"); printCentered("Goodbye!"); exit(0);
            default: printf("\n"); printCentered("Invalid option!");
        }
    } else {
        switch (choice) {
            case 1:
                loadUsers(users, &userCount); loadPlates(plates, &plateCount); loadRequests(requests, &requestCount);
                showDashboard();
                printf("\n                    Press Enter to return..."); getchar();
                break;
            case 2: userMenu(g_loggedInUser, &g_loggedIn); break;
            case 3: plateMenu(g_loggedInUser); break;
            case 4: requestMenu(g_loggedInUser); break;
            case 5: chatMenu(g_loggedInUser); break;
            case 6: showSettings(); break;
            case 7: g_loggedIn = 0; strcpy(g_loggedInUser, ""); printCentered("Logged out."); printf("\n                    Press Enter..."); getchar(); break;
            case 8: printf("\n"); printCentered("Goodbye!"); exit(0);
            default: printf("\n"); printCentered("Invalid option!");
        }
    }
}

void runMenuSystem(void) {
    int choice = 0;
    while (1) {
        displayMenu();
        if (scanf("%d", &choice) != 1) {
            printf("\n"); printCentered("Please enter a valid number!");
            while (getchar() != '\n');
            printf("\n                    Press Enter..."); getchar();
            continue;
        }
        getchar();
        handleMenuChoice(choice);
    }
}