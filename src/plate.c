/**
 * @file plate.c
 * @brief Plate management implementation
 * @author Member 3
 */

#include "plate.h"
#include "chat.h"
#include "core.h"
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
        if (sscanf(line, "%d,%[^,],%[^,],%[^,],%d,%[^,],%[^,],%s",
            &plates[*count].id, plates[*count].donor, plates[*count].foodName,
            plates[*count].description, &plates[*count].quantity,
            plates[*count].expiryDate, plates[*count].dateAdded,
            plates[*count].status) == 8) (*count)++;
        else if (sscanf(line, "%d,%[^,],%[^,],%[^,],%d,%[^,],%s",
            &plates[*count].id, plates[*count].donor, plates[*count].foodName,
            plates[*count].description, &plates[*count].quantity,
            plates[*count].expiryDate, plates[*count].status) == 7) {
            /* Backward compatibility: old format without dateAdded */
            strcpy(plates[*count].dateAdded, "2026-01-01 00:00");
            (*count)++;
        }
    }
    fclose(fp);
}

void savePlates(const Plate *plates, int count) {
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "%splates.csv", appConfig.dataFolder);
    FILE *fp = fopen(path, "w");
    if (fp == NULL) return;
    for (int i = 0; i < count; i++)
        fprintf(fp, "%d,%s,%s,%s,%d,%s,%s,%s\n", plates[i].id, plates[i].donor,
            plates[i].foodName, plates[i].description, plates[i].quantity,
            plates[i].expiryDate, plates[i].dateAdded, plates[i].status);
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

int isDateInPast(const char *date) {
    char today[11];
    getCurrentDate(today);
    return strcmp(date, today) < 0;
}

/**
 * @brief Validates quantity input, re-asks on invalid
 */
static int getValidQuantity(void) {
    char qtyStr[20];
    int qty;
    while (1) {
        printf("Quantity: ");
        fgets(qtyStr, sizeof(qtyStr), stdin);
        qtyStr[strcspn(qtyStr, "\n")] = 0;
        if (strlen(qtyStr) == 0) {
            printf("[Warning] Quantity cannot be empty! Please enter a positive number.\n");
            continue;
        }
        if (sscanf(qtyStr, "%d", &qty) != 1 || qty <= 0) {
            printf("[Error] Invalid quantity! Must be a positive number.\n");
            continue;
        }
        return qty;
    }
}

/**
 * @brief Validates expiry date input, re-asks on invalid or past date
 */
static void getValidExpiryDate(char *expiryDate) {
    while (1) {
        printf("Expiry Date (YYYY-MM-DD): ");
        fgets(expiryDate, 11, stdin);
        expiryDate[strcspn(expiryDate, "\n")] = 0;
        if (strlen(expiryDate) == 0) {
            printf("[Warning] Date cannot be empty!\n");
            continue;
        }
        if (!validateDate(expiryDate)) {
            printf("[Error] Invalid date format! Use YYYY-MM-DD.\n");
            continue;
        }
        if (isDateInPast(expiryDate)) {
            printf("[Rejected] This date is in the past! Food is expired. Please enter a future date.\n");
            continue;
        }
        break;
    }
}

int addPlate(Plate *plates, int *count, const char *donor) {
    if (*count >= MAX_PLATES) {
        printf("[Error] Plate database is full!\n");
        return 0;
    }
    Plate p;
    p.id = (*count > 0) ? plates[*count - 1].id + 1 : 1;
    strcpy(p.donor, donor);

    /* Food name - required */
    while (1) {
        printf("Food name (required): ");
        fgets(p.foodName, 40, stdin);
        p.foodName[strcspn(p.foodName, "\n")] = 0;
        if (strlen(p.foodName) > 0) break;
        printf("[Warning] Food name cannot be empty! Please enter the food name.\n");
    }

    /* Description - optional, can skip with Enter */
    printf("Description (press Enter to skip): ");
    fgets(p.description, 100, stdin);
    p.description[strcspn(p.description, "\n")] = 0;
    if (strlen(p.description) == 0) {
        strcpy(p.description, "(No description)");
    }

    /* Quantity - required, validated */
    p.quantity = getValidQuantity();

    /* Expiry date - required, validated, checks past dates */
    getValidExpiryDate(p.expiryDate);

    /* Timestamp */
    getCurrentTimestamp(p.dateAdded, sizeof(p.dateAdded));
    strcpy(p.status, "Available");

    plates[*count] = p;
    (*count)++;
    savePlates(plates, *count);

    printf("\n[Success] Plate added successfully!\n");
    printf("  ID: %d | Food: %s | Qty: %d | Expiry: %s\n", p.id, p.foodName, p.quantity, p.expiryDate);

    /* Notify all users */
    char notifMsg[MAX_MSG];
    snprintf(notifMsg, sizeof(notifMsg), "New plate available: %s from %s (Qty: %d, Expires: %s)",
        p.foodName, donor, p.quantity, p.expiryDate);
    /* Send to all registered users */
    for (int i = 0; i < userCount; i++) {
        addNotification(users[i].username, notifMsg);
    }
    addNotification("all", notifMsg);
    return 1;
}

void displayAllPlates(const Plate *plates, int count) {
    if (count == 0) { printf("\nNo plates available yet.\n"); return; }
    printf("\n%-5s %-15s %-20s %-8s %-12s %-15s %-15s\n",
        "ID", "Donor", "Food", "Qty", "Expiry", "Status", "Added");
    printf("--------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < count; i++)
        printf("%-5d %-15s %-20s %-8d %-12s %-15s %-15s\n",
            plates[i].id, plates[i].donor, plates[i].foodName, plates[i].quantity,
            plates[i].expiryDate, plates[i].status, plates[i].dateAdded);
}

void displayMyPlates(const Plate *plates, int count, const char *donor) {
    int found = 0;
    printf("\n%-5s %-20s %-8s %-12s %-15s %-15s\n",
        "ID", "Food", "Qty", "Expiry", "Status", "Added");
    printf("--------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        if (strcmp(plates[i].donor, donor) == 0) {
            printf("%-5d %-20s %-8d %-12s %-15s %-15s\n",
                plates[i].id, plates[i].foodName, plates[i].quantity,
                plates[i].expiryDate, plates[i].status, plates[i].dateAdded);
            found++;
        }
    }
    if (!found) printf("You haven't donated any plates yet.\n");
}

int deleteMyPlate(Plate *plates, int *count, const char *donor) {
    displayMyPlates(plates, *count, donor);
    int pid;
    printf("\nEnter plate ID to delete (0 to cancel): ");
    if (scanf("%d", &pid) != 1) { printf("[Error] Invalid input.\n"); while (getchar() != '\n'); return 0; }
    getchar();
    if (pid == 0) { printf("Deletion cancelled.\n"); return 0; }
    for (int i = 0; i < *count; i++) {
        if (plates[i].id == pid && strcmp(plates[i].donor, donor) == 0) {
            if (strcmp(plates[i].status, "Available") != 0) {
                printf("[Error] Can only delete plates with 'Available' status.\n");
                return 0;
            }
            for (int j = i; j < *count - 1; j++) plates[j] = plates[j + 1];
            (*count)--;
            savePlates(plates, *count);
            printf("[Success] Plate #%d deleted.\n", pid);
            return 1;
        }
    }
    printf("[Error] Plate not found or not yours.\n");
    return 0;
}

int searchPlateByName(const Plate *plates, int count, const char *food) {
    int found = 0;
    char fl[40], pl[40];
    strcpy(fl, food); for (int i = 0; fl[i]; i++) fl[i] = tolower(fl[i]);
    printf("\n%-5s %-15s %-20s %-8s %-12s %-15s\n", "ID", "Donor", "Food", "Qty", "Expiry", "Status");
    printf("--------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        strcpy(pl, plates[i].foodName); for (int j = 0; pl[j]; j++) pl[j] = tolower(pl[j]);
        if (strstr(pl, fl)) {
            printf("%-5d %-15s %-20s %-8d %-12s %-15s\n",
                plates[i].id, plates[i].donor, plates[i].foodName,
                plates[i].quantity, plates[i].expiryDate, plates[i].status);
            found++;
        }
    }
    if (!found) printf("No plates found matching '%s'.\n", food);
    return found;
}

int searchPlateByDonor(const Plate *plates, int count, const char *donor) {
    int found = 0;
    char dl[30], pl[30];
    strcpy(dl, donor); for (int i = 0; dl[i]; i++) dl[i] = tolower(dl[i]);
    printf("\n%-5s %-15s %-20s %-8s %-12s %-15s\n", "ID", "Donor", "Food", "Qty", "Expiry", "Status");
    printf("--------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        strcpy(pl, plates[i].donor); for (int j = 0; pl[j]; j++) pl[j] = tolower(pl[j]);
        if (strstr(pl, dl)) {
            printf("%-5d %-15s %-20s %-8d %-12s %-15s\n",
                plates[i].id, plates[i].donor, plates[i].foodName,
                plates[i].quantity, plates[i].expiryDate, plates[i].status);
            found++;
        }
    }
    if (!found) printf("No plates found from donor '%s'.\n", donor);
    return found;
}

void sortPlatesByExpiry(Plate *plates, int count) {
    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - i - 1; j++)
            if (strcmp(plates[j].expiryDate, plates[j + 1].expiryDate) > 0) {
                Plate t = plates[j]; plates[j] = plates[j + 1]; plates[j + 1] = t;
            }
    printf("[Sorted by: Expiry Date (oldest first)]\n");
}

void sortPlatesByDonor(Plate *plates, int count) {
    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - i - 1; j++)
            if (strcmp(plates[j].donor, plates[j + 1].donor) > 0) {
                Plate t = plates[j]; plates[j] = plates[j + 1]; plates[j + 1] = t;
            }
    printf("[Sorted by: Donor Name (A-Z)]\n");
}

void sortPlatesByStatus(Plate *plates, int count) {
    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - i - 1; j++)
            if (strcmp(plates[j].status, plates[j + 1].status) > 0) {
                Plate t = plates[j]; plates[j] = plates[j + 1]; plates[j + 1] = t;
            }
    printf("[Sorted by: Status (A-Z)]\n");
}

void filterAndSortPlates(Plate *plates, int count) {
    int choice;
    while (1) {
        system(CLEAR_SCREEN);
        printf("\n");
        printCenteredLine('=', 36);
        printCentered("FILTER & SORT PLATES");
        printCenteredLine('=', 36);
        printf("                    1. Sort by Expiry Date (Oldest First)\n");
        printf("                    2. Sort by Donor Name (A-Z)\n");
        printf("                    3. Sort by Status (A-Z)\n");
        printf("                    4. Search by Food Name\n");
        printf("                    5. Search by Donor Name\n");
        printf("                    6. Back\n");
        printCenteredLine('=', 36);
        printf("                    Choice: ");
        scanf("%d", &choice); getchar();

        char input[MAX_LINE];
        switch (choice) {
            case 1: sortPlatesByExpiry(plates, count); displayAllPlates(plates, count); break;
            case 2: sortPlatesByDonor(plates, count); displayAllPlates(plates, count); break;
            case 3: sortPlatesByStatus(plates, count); displayAllPlates(plates, count); break;
            case 4:
                printf("Search food name: ");
                fgets(input, sizeof(input), stdin); input[strcspn(input, "\n")] = 0;
                searchPlateByName(plates, count, input);
                break;
            case 5:
                printf("Search donor name: ");
                fgets(input, sizeof(input), stdin); input[strcspn(input, "\n")] = 0;
                searchPlateByDonor(plates, count, input);
                break;
            case 6: return;
            default: printf("\n"); printCentered("Invalid choice.");
        }
        if (choice >= 1 && choice <= 5) {
            printf("\n                    Press Enter to continue..."); getchar();
        }
    }
}

void plateMenu(const char *loggedInUser) {
    int choice;
    while (1) {
        system(CLEAR_SCREEN);
        printf("\n");
        printCenteredLine('=', 36);
        printCentered("PLATE MANAGEMENT");
        printCenteredLine('=', 36);
        printf("                    1. Add New Plate\n");
        printf("                    2. View All Plates\n");
        printf("                    3. View My Plates\n");
        printf("                    4. Delete My Plate\n");
        printf("                    5. Filter & Sort Plates\n");
        printf("                    6. Back to Main Menu\n");
        printCenteredLine('=', 36);
        printf("                    Choice: ");
        scanf("%d", &choice); getchar();

        switch (choice) {
            case 1:
                system(CLEAR_SCREEN);
                printf("\n--- Add New Plate ---\n");
                addPlate(plates, &plateCount, loggedInUser);
                printf("\n                    Press Enter to continue..."); getchar();
                break;
            case 2:
                system(CLEAR_SCREEN);
                loadPlates(plates, &plateCount);
                displayAllPlates(plates, plateCount);
                printf("\n                    Press Enter to continue..."); getchar();
                break;
            case 3:
                system(CLEAR_SCREEN);
                loadPlates(plates, &plateCount);
                displayMyPlates(plates, plateCount, loggedInUser);
                printf("\n                    Press Enter to continue..."); getchar();
                break;
            case 4:
                system(CLEAR_SCREEN);
                loadPlates(plates, &plateCount);
                deleteMyPlate(plates, &plateCount, loggedInUser);
                printf("\n                    Press Enter to continue..."); getchar();
                break;
            case 5:
                loadPlates(plates, &plateCount);
                filterAndSortPlates(plates, plateCount);
                break;
            case 6: return;
            default: printf("\n"); printCentered("Invalid choice.");
        }
    }
}