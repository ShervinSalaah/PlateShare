/**
 * @file plate.c
 * @brief Food sharing (plate) management implementation
 * @author 254186E RUBASINHEGE S.N
 * 
 * Handles all food donation operations:
 * - Adding new food plates with pickup/delivery options
 * - Viewing all available food or only your own
 * - Searching by food name or donor
 * - Filtering and sorting plates
 * - Deleting your own plates
 * - Sending notifications to all users when new food is added
 */

#include "plate.h"
#include "chat.h"
#include "core.h"
#include <ctype.h>

/**
 * @brief Loads plates from plates.csv into memory
 * 
 * Supports both old format (without dateAdded/address) and new format.
 */
void loadPlates(Plate *plates, int *count) {
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "%splates.csv", appConfig.dataFolder);
    FILE *fp = fopen(path, "r");
    if (fp == NULL) { *count = 0; return; }
    *count = 0;
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp) && *count < MAX_PLATES) {
        line[strcspn(line, "\n")] = 0;
        /* Try new format first (8 fields) */
        if (sscanf(line, "%d,%[^,],%[^,],%[^,],%d,%[^,],%[^,],%[^,],%[^,],%s",
            &plates[*count].id, plates[*count].donor, plates[*count].foodName,
            plates[*count].description, &plates[*count].quantity,
            plates[*count].expiryDate, plates[*count].dateAdded,
            plates[*count].pickupOption, plates[*count].address,
            plates[*count].status) == 10) {
            (*count)++;
        }
        /* Try old format (7 fields) */
        else if (sscanf(line, "%d,%[^,],%[^,],%[^,],%d,%[^,],%s",
            &plates[*count].id, plates[*count].donor, plates[*count].foodName,
            plates[*count].description, &plates[*count].quantity,
            plates[*count].expiryDate, plates[*count].status) == 7) {
            strcpy(plates[*count].dateAdded, "2026-01-01 00:00");
            strcpy(plates[*count].pickupOption, "Self Pickup");
            strcpy(plates[*count].address, "(No address)");
            (*count)++;
        }
    }
    fclose(fp);
}

/**
 * @brief Saves plates to plates.csv
 */
void savePlates(const Plate *plates, int count) {
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "%splates.csv", appConfig.dataFolder);
    FILE *fp = fopen(path, "w");
    if (fp == NULL) return;
    for (int i = 0; i < count; i++)
        fprintf(fp, "%d,%s,%s,%s,%d,%s,%s,%s,%s,%s\n",
            plates[i].id, plates[i].donor, plates[i].foodName,
            plates[i].description, plates[i].quantity,
            plates[i].expiryDate, plates[i].dateAdded,
            plates[i].pickupOption, plates[i].address,
            plates[i].status);
    fclose(fp);
}

/**
 * @brief Checks if a date string is in valid YYYY-MM-DD format
 */
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

/**
 * @brief Checks if a date is before today (expired)
 */
int isDateInPast(const char *date) {
    char today[11];
    getCurrentDate(today);
    return strcmp(date, today) < 0;
}

/**
 * @brief Gets a valid quantity from the user (re-asks on invalid input)
 */
static int getValidQuantity(void) {
    char qtyStr[20];
    int qty;
    while (1) {
        printf("  How many portions? (0 to cancel): ");

        fgets(qtyStr, sizeof(qtyStr), stdin);
        qtyStr[strcspn(qtyStr, "\n")] = 0;
        if (strlen(qtyStr) == 0) {
            printf("  Please enter a number. Cannot be empty.\n");
            continue;
        }
        if (sscanf(qtyStr, "%d", &qty) != 1 || qty <= 0) {
                if (qty == 0) 
                    return 0;
            printf("  Please enter a valid positive number.\n");
            continue;
        }
        return qty;
    }
}

/**
 * @brief Gets a valid expiry date (re-asks on invalid or past date)
 */
static int getValidExpiryDate(char *expiryDate) {
    while (1) {
        printf("  Expiry date (YYYY-MM-DD) or '0' to cancel: ");
        fgets(expiryDate, 11, stdin);
        expiryDate[strcspn(expiryDate, "\n")] = 0;
        if (strcmp(expiryDate, "0") == 0) 
            return 0;
        if (strlen(expiryDate) == 0) {
            printf("  Date cannot be empty. Please enter a date.\n");
            continue;
        }
        if (!validateDate(expiryDate)) {
            printf("  Wrong format! Use YYYY-MM-DD (example: 2026-12-31).\n");
            continue;
        }
        if (isDateInPast(expiryDate)) {
            printf("  This date has already passed! Please enter a future date.\n");
            continue;
        }
        break;
    }
        return 1; 

}

/**
 * @brief Adds a new food plate to the system
 * 
 * Guides the user through entering food details.
 * Required fields: food name, quantity, expiry date.
 * Optional: description (can skip).
 * New: pickup/delivery option with address.
 * 
 * Sends a notification to ALL users about the new food.
 */
int addPlate(Plate *plates, int *count, const char *donor) {
    if (*count >= MAX_PLATES) {
        printf("\n  Sorry! Food list is full. Cannot add more.\n");
        pauseScreen("");
        return 0;
    }
    Plate p;
    p.id = (*count > 0) ? plates[*count - 1].id + 1 : 1;
    strcpy(p.donor, donor);

    printf("\n--- Share Your Food ---\n");
    printf("  (Type '0' at any prompt to cancel)\n\n");

    /* Food name - required */
    while (1) {
        printf("  Name of the food (required): ");
        fgets(p.foodName, 40, stdin);
        p.foodName[strcspn(p.foodName, "\n")] = 0;
        if (strcmp(p.foodName, "0") == 0) { 
                    printf("  Cancelled.\n"); pauseScreen(""); return 0; 
            }
        if (strlen(p.foodName) > 0) break;
        printf("  Food name cannot be empty. Please enter what you are sharing.\n");
    }

    /* Description - optional */
    printf("  Description (press Enter to skip): ");
    fgets(p.description, 100, stdin);
    p.description[strcspn(p.description, "\n")] = 0;
    if (strcmp(p.description, "0") == 0) { 
            printf("  Cancelled.\n"); pauseScreen(""); return 0; 
        }
    if (strlen(p.description) == 0) {
        strcpy(p.description, "(No description)");
    }

    /* Quantity - required */
    p.quantity = getValidQuantity();

    /* Expiry date - required, validated, past dates rejected */
    getValidExpiryDate(p.expiryDate);

    /* Pickup option - required */
    int pickupChoice;
printf("\n  How should people get this food?\n");
printf("    1. Self Pickup (they come to you)\n");
printf("    2. Open to Delivery (you can bring it to them)\n");

while (1) {
    printf("  Your choice (1 or 2, 0 to cancel): ");
    char choiceStr[10];
    fgets(choiceStr, sizeof(choiceStr), stdin);
    choiceStr[strcspn(choiceStr, "\n")] = 0;
    
    if (strlen(choiceStr) == 0 || sscanf(choiceStr, "%d", &pickupChoice) != 1) {
        printf("  Please enter 1 or 2.\n");
        continue;
    }
    
    if (strcmp(choiceStr, "0") == 0) { 
        printf("  Cancelled.\n"); 
        pauseScreen(""); 
        return 0; 
    }
    
    if (pickupChoice == 1) { 
        strcpy(p.pickupOption, "Self Pickup"); 
        break; 
    }
    if (pickupChoice == 2) { 
        strcpy(p.pickupOption, "Open to Delivery"); 
        break;
    }
    
    printf("  Please enter 1 or 2.\n");
}
    /* Address - required */
    while (1) {
        printf("  Pickup address or preferred delivery area: ");
        fgets(p.address, MAX_ADDRESS, stdin);
        p.address[strcspn(p.address, "\n")] = 0;
        if (strlen(p.address) > 0) break;
        printf("  Address cannot be empty. Please enter a location.\n");
    }

    /* Timestamp and status */
    getCurrentTimestamp(p.dateAdded, sizeof(p.dateAdded));
    strcpy(p.status, "Available");

    /* Save */
    plates[*count] = p;
    (*count)++;
    savePlates(plates, *count);

    printf("\n  Food shared successfully!\n");
    printf("  ID: %d | Food: %s | Portions: %d | Expires: %s\n", p.id, p.foodName, p.quantity, p.expiryDate);
    printf("  Pickup: %s | Location: %s\n", p.pickupOption, p.address);

    /* Notify all users about the new food */
    char notifMsg[MAX_MSG];
    snprintf(notifMsg, sizeof(notifMsg), "New food: %s from %s (%d portions, %s, %s)",
        p.foodName, donor, p.quantity, p.pickupOption, p.expiryDate);
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, donor) != 0) {
            addNotification(users[i].username, notifMsg);
        }
    }
    pauseScreen("");
    return 1;
}

/**
 * @brief Shows all plates in the system
 */
void displayAllPlates(const Plate *plates, int count) {
    if (count == 0) {
        printf("\n  No food shared yet.\n");
        return;
    }
    printf("\n  All Shared Food:\n\n");
    printf("  %-5s %-15s %-20s %-6s %-12s %-15s %-15s\n",
        "ID", "Shared By", "Food", "Qty", "Expires", "Pickup", "Status");
    printf("  --------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < count; i++)
        printf("  %-5d %-15s %-20s %-6d %-12s %-15s %-15s\n",
            plates[i].id, plates[i].donor, plates[i].foodName, plates[i].quantity,
            plates[i].expiryDate, plates[i].pickupOption, plates[i].status);
}

/**
 * @brief Shows only plates donated by the logged-in user
 */
void displayMyPlates(const Plate *plates, int count, const char *donor) {
    int found = 0;
    printf("\n  Food You Shared:\n\n");
    printf("  %-5s %-20s %-6s %-12s %-15s %-15s\n",
        "ID", "Food", "Qty", "Expires", "Pickup", "Status");
    printf("  --------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        if (strcmp(plates[i].donor, donor) == 0) {
            printf("  %-5d %-20s %-6d %-12s %-15s %-15s\n",
                plates[i].id, plates[i].foodName, plates[i].quantity,
                plates[i].expiryDate, plates[i].pickupOption, plates[i].status);
            found++;
        }
    }
    if (!found) printf("  You haven't shared any food yet.\n");
}

/**
 * @brief Deletes one of the user's own plates (only if still Available)
 */
int deleteMyPlate(Plate *plates, int *count, const char *donor) {
    displayMyPlates(plates, *count, donor);
    int pid;
    printf("\n  Enter the ID of food to remove (0 to cancel): ");
    if (scanf("%d", &pid) != 1) {
        printf("  Invalid input.\n");
        while (getchar() != '\n');
        pauseScreen("");
        return 0;
    }
    getchar();
    if (pid == 0) { printf("  Cancelled.\n"); pauseScreen(""); return 0; }

    for (int i = 0; i < *count; i++) {
        if (plates[i].id == pid && strcmp(plates[i].donor, donor) == 0) {
            if (strcmp(plates[i].status, "Available") != 0) {
                printf("  You can only remove food that is still Available.\n");
                pauseScreen("");
                return 0;
            }
            for (int j = i; j < *count - 1; j++) plates[j] = plates[j + 1];
            (*count)--;
            savePlates(plates, *count);
            printf("  Food #%d has been removed.\n", pid);
            pauseScreen("");
            return 1;
        }
    }
    printf("  Food not found or not yours.\n");
    pauseScreen("");
    return 0;
}

/**
 * @brief Searches plates by food name (case-insensitive)
 */
int searchPlateByName(const Plate *plates, int count, const char *food) {
    int found = 0;
    char fl[40], pl[40];
    strcpy(fl, food); for (int i = 0; fl[i]; i++) fl[i] = tolower(fl[i]);
    printf("\n  Search Results for '%s':\n\n", food);
    printf("  %-5s %-15s %-20s %-6s %-12s %-15s\n", "ID", "Shared By", "Food", "Qty", "Expires", "Pickup");
    printf("  --------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        strcpy(pl, plates[i].foodName); for (int j = 0; pl[j]; j++) pl[j] = tolower(pl[j]);
        if (strstr(pl, fl)) {
            printf("  %-5d %-15s %-20s %-6d %-12s %-15s\n",
                plates[i].id, plates[i].donor, plates[i].foodName,
                plates[i].quantity, plates[i].expiryDate, plates[i].pickupOption);
            found++;
        }
    }
    if (!found) printf("  No food found matching '%s'.\n", food);
    return found;
}

/**
 * @brief Searches plates by donor name (case-insensitive)
 */
int searchPlateByDonor(const Plate *plates, int count, const char *donor) {
    int found = 0;
    char dl[30], pl[30];
    strcpy(dl, donor); for (int i = 0; dl[i]; i++) dl[i] = tolower(dl[i]);
    printf("\n  Food shared by '%s':\n\n", donor);
    printf("  %-5s %-20s %-6s %-12s %-15s\n", "ID", "Food", "Qty", "Expires", "Pickup");
    printf("  ----------------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        strcpy(pl, plates[i].donor); for (int j = 0; pl[j]; j++) pl[j] = tolower(pl[j]);
        if (strstr(pl, dl)) {
            printf("  %-5d %-20s %-6d %-12s %-15s\n",
                plates[i].id, plates[i].foodName, plates[i].quantity,
                plates[i].expiryDate, plates[i].pickupOption);
            found++;
        }
    }
    if (!found) printf("  No food found from '%s'.\n", donor);
    return found;
}

void sortPlatesByExpiry(Plate *plates, int count) {
    /* Remove past dates by shifting valid plates forward */
    int validCount = 0;
    for (int i = 0; i < count; i++) {
        if (!isDateInPast(plates[i].expiryDate)) {
            if (i != validCount) plates[validCount] = plates[i];
            validCount++;
        }
    }
    /* Sort remaining valid plates */
    for (int i = 0; i < validCount - 1; i++)
        for (int j = 0; j < validCount - i - 1; j++)
            if (strcmp(plates[j].expiryDate, plates[j + 1].expiryDate) > 0) {
                Plate t = plates[j]; plates[j] = plates[j + 1]; plates[j + 1] = t;
            }
    printf("  [Sorted: Expiring Soon First | Past dates removed]\n");
}
void sortPlatesByDonor(Plate *plates, int count) {
    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - i - 1; j++)
            if (strcmp(plates[j].donor, plates[j + 1].donor) > 0) {
                Plate t = plates[j]; plates[j] = plates[j + 1]; plates[j + 1] = t;
            }
    printf("  [Sorted: By Donor Name A-Z]\n");
}

void sortPlatesByPickup(Plate *plates, int count) {
    for (int i = 0; i < count - 1; i++)
        for (int j = 0; j < count - i - 1; j++)
            if (strcmp(plates[j].pickupOption, plates[j + 1].pickupOption) > 0) {
                Plate t = plates[j]; plates[j] = plates[j + 1]; plates[j + 1] = t;
            }
    printf("  [Sorted: By Pickup Method]\n");
}

/**
 * @brief Filter and sort plates menu
 * 
 * Provides options to sort by expiry, donor, or pickup method,
 * or search by food name or donor name.
 */
void filterAndSortPlates(Plate *plates, int count) {
    int choice;
    while (1) {
        system(CLEAR_SCREEN);
        printf("\n");
        printCenteredLine('=', 40);
        printCentered("FIND & SORT FOOD");
        printCenteredLine('=', 40);
        printf("                    1. Sort: Expiring Soon First\n");
        printf("                    2. Sort: By Donor Name (A-Z)\n");
        printf("                    3. Sort: By Pickup Method\n");
        printf("                    4. Search by Food Name\n");
        printf("                    5. Search by Donor Name\n");
        printf("                    6. Go Back\n");
        printCenteredLine('=', 40);
        printf("                    Your choice: ");
        scanf("%d", &choice); getchar();

        char input[MAX_LINE];
        switch (choice) {
            case 1: sortPlatesByExpiry(plates, count); displayAllPlates(plates, count); break;
            case 2: sortPlatesByDonor(plates, count); displayAllPlates(plates, count); break;
            case 3: sortPlatesByPickup(plates, count); displayAllPlates(plates, count); break;
            case 4:
                printf("  Search food name (0 to cancel): ");
                fgets(input, sizeof(input), stdin); input[strcspn(input, "\n")] = 0;
                if (strcmp(input, "0") != 0) {
                    searchPlateByName(plates, count, input);
                }
                break;
            case 5:
                printf("  Search donor name (0 to cancel): ");
                fgets(input, sizeof(input), stdin); input[strcspn(input, "\n")] = 0;
                if (strcmp(input, "0") != 0) {
                    searchPlateByDonor(plates, count, input);
                }
                break;
            case 6: return;
            default: printf("\n"); printCentered("Wrong choice!"); pauseScreen("");
        }
        if (choice >= 1 && choice <= 5) pauseScreen("");
    }
}

/**
 * @brief Food sharing sub-menu
 */
void plateMenu(const char *loggedInUser) {
    int choice;
    while (1) {
        system(CLEAR_SCREEN);
        printf("\n");
        printCenteredLine('=', 40);
        printCentered("SHARE FOOD");
        printCenteredLine('=', 40);
        printf("  1. Share New Food       - List your extra food\n");
        printf("  2. See All Food         - Browse all available food\n");
        printf("  3. See My Food          - View your own listings\n");
        printf("  4. Remove My Food       - Delete your listing\n");
        printf("  5. Find & Sort Food     - Search and organize\n");
        printf("  6. Go Back\n");
        printCenteredLine('=', 40);
        printf("                    Your choice: ");
        scanf("%d", &choice); getchar();

        switch (choice) {
            case 1:
                system(CLEAR_SCREEN);
                addPlate(plates, &plateCount, loggedInUser);
                break;
            case 2:
                system(CLEAR_SCREEN);
                loadPlates(plates, &plateCount);
                displayAllPlates(plates, plateCount);
                pauseScreen("");
                break;
            case 3:
                system(CLEAR_SCREEN);
                loadPlates(plates, &plateCount);
                displayMyPlates(plates, plateCount, loggedInUser);
                pauseScreen("");
                break;
            case 4:
                system(CLEAR_SCREEN);
                loadPlates(plates, &plateCount);
                deleteMyPlate(plates, &plateCount, loggedInUser);
                break;
            case 5:
                loadPlates(plates, &plateCount);
                filterAndSortPlates(plates, plateCount);
                break;
            case 6: return;
            default: printf("\n"); printCentered("Wrong choice!"); pauseScreen("");
        }
    }
}