#include "core.h"

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
    printf("PlateShare Pro initialized.\n");
}

int displayMainMenu() {
    int choice;
    printf("\n=================================\n");
    printf("       PLATESHARE PRO\n");
    printf("=================================\n");
    printf("1. User Management\n");
    printf("2. Plate Management\n");
    printf("3. Request Management\n");
    printf("4. Chat Room\n");
    printf("5. Settings\n");
    printf("6. Exit\n");
    printf("=================================\n");
    printf("Choose an option: ");
    scanf("%d", &choice);
    getchar();
    return choice;
}

void loadConfig(Config *cfg) {
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "%sconfig.ini", cfg->dataFolder);
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        saveConfig(cfg);
        return;
    }
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