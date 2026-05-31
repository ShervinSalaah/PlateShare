#include "plate.h"
#include "chat.h"
#include <ctype.h>

void loadPlates(Plate *plates, int *count) {
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "%splates.csv", appConfig.dataFolder);
    FILE *fp = fopen(path, "r");
    if (fp == NULL) { *count = 0; return; }
    *count = 0;
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp) && *count < MAX_PLATES) {
        line[strcspn(line, "\n")] = 0;
        if (sscanf(line, "%d,%[^,],%[^,],%[^,],%d,%[^,],%s",
            &plates[*count].id, plates[*count].donor, plates[*count].foodName,
            plates[*count].description, &plates[*count].quantity,
            plates[*count].expiryDate, plates[*count].status) == 7) (*count)++;
    }
    fclose(fp);
}

void savePlates(const Plate *plates, int count) {
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "%splates.csv", appConfig.dataFolder);
    FILE *fp = fopen(path, "w");
    if (fp == NULL) return;
    for (int i = 0; i < count; i++)
        fprintf(fp, "%d,%s,%s,%s,%d,%s,%s\n", plates[i].id, plates[i].donor,
            plates[i].foodName, plates[i].description, plates[i].quantity,
            plates[i].expiryDate, plates[i].status);
    fclose(fp);
}

int validateDate(const char *date) {
    if (strlen(date) != 10) return 0;
    if (date[4] != '-' || date[7] != '-') return 0;
    int y, m, d;
    if (sscanf(date, "%d-%d-%d", &y, &m, &d) != 3) return 0;
    if (y < 2024 || m < 1 || m > 12 || d < 1 || d > 31) return 0;
    if (m == 2 && d > 29) return 0;
    if ((m == 4 || m == 6 || m == 9 || m == 11) && d > 30) return 0;
    return 1;
}

int addPlate(Plate *plates, int *count, const char *donor) {
    if (*count >= MAX_PLATES) { printf("Plate list full.\n"); return 0; }
    Plate p;
    p.id = (*count > 0) ? plates[*count - 1].id + 1 : 1;
    strcpy(p.donor, donor);
    printf("Food name: "); fgets(p.foodName, 40, stdin); p.foodName[strcspn(p.foodName, "\n")] = 0;
    printf("Description: "); fgets(p.description, 100, stdin); p.description[strcspn(p.description, "\n")] = 0;
    printf("Quantity: "); scanf("%d", &p.quantity); getchar();
    printf("Expiry (YYYY-MM-DD): "); fgets(p.expiryDate, 11, stdin); p.expiryDate[strcspn(p.expiryDate, "\n")] = 0;
    if (!validateDate(p.expiryDate)) { printf("Invalid date.\n"); return 0; }
    strcpy(p.status, "Available");
    plates[*count] = p; (*count)++;
    savePlates(plates, *count);
    printf("Plate added (ID: %d).\n", p.id);
    
      /* Notify all users about new plate */
    char notifMsg[MAX_MSG];
    snprintf(notifMsg, sizeof(notifMsg), "%s added a new plate: %s (Qty: %d)", donor, p.foodName, p.quantity);
    addNotification("all", notifMsg);
    
    return 1;
}

void displayAllPlates(const Plate *plates, int count) {
    printf("\n%-5s %-15s %-20s %-10s %-12s %-15s\n", "ID", "Donor", "Food", "Qty", "Expiry", "Status");
    for (int i = 0; i < count; i++)
        printf("%-5d %-15s %-20s %-10d %-12s %-15s\n", plates[i].id, plates[i].donor,
            plates[i].foodName, plates[i].quantity, plates[i].expiryDate, plates[i].status);
}

int searchPlateByName(const Plate *plates, int count, const char *food) {
    int found = 0;
    char fl[40], pl[40];
    strcpy(fl, food); for (int i = 0; fl[i]; i++) fl[i] = tolower(fl[i]);
    for (int i = 0; i < count; i++) {
        strcpy(pl, plates[i].foodName); for (int j = 0; pl[j]; j++) pl[j] = tolower(pl[j]);
        if (strstr(pl, fl)) {
            printf("%-5d %-15s %-20s %-10d %-12s %-15s\n", plates[i].id, plates[i].donor,
                plates[i].foodName, plates[i].quantity, plates[i].expiryDate, plates[i].status);
            found++;
        }
    }
    if (!found) printf("No plates found.\n");
    return found;
}

void sortPlatesByExpiry(Plate *plates, int count) {
    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - i - 1; j++)
            if (strcmp(plates[j].expiryDate, plates[j + 1].expiryDate) > 0) {
                Plate t = plates[j]; plates[j] = plates[j + 1]; plates[j + 1] = t;
            }
    printf("Sorted by expiry.\n");
}