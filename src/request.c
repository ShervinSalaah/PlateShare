/**
 * @file request.c
 * @brief Request and transaction management implementation
 * @author Member 4
 */

#include "request.h"
#include "chat.h"
#include "core.h"

void loadRequests(Request *requests, int *count) {
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "%srequests.csv", appConfig.dataFolder);
    FILE *fp = fopen(path, "r");
    if (fp == NULL) { *count = 0; return; }
    *count = 0;
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp) && *count < MAX_REQUESTS) {
        line[strcspn(line, "\n")] = 0;
        if (sscanf(line, "%d,%d,%[^,],%[^,],%s",
            &requests[*count].id, &requests[*count].plateId,
            requests[*count].requester, requests[*count].status,
            requests[*count].timestamp) == 5) (*count)++;
    }
    fclose(fp);
}

void saveRequests(const Request *requests, int count) {
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "%srequests.csv", appConfig.dataFolder);
    FILE *fp = fopen(path, "w");
    if (fp == NULL) return;
    for (int i = 0; i < count; i++)
        fprintf(fp, "%d,%d,%s,%s,%s\n", requests[i].id, requests[i].plateId,
            requests[i].requester, requests[i].status, requests[i].timestamp);
    fclose(fp);
}

int createRequest(Request *requests, int *reqCount, Plate *plates, int plateCount, const char *requester) {
    printf("\n--- Available Plates ---\n");
    int availableFound = 0;
    for (int i = 0; i < plateCount; i++) {
        if (strcmp(plates[i].status, "Available") == 0) {
            printf("ID: %d | %s | Donor: %s | Qty: %d | Expiry: %s\n",
                plates[i].id, plates[i].foodName, plates[i].donor,
                plates[i].quantity, plates[i].expiryDate);
            availableFound++;
        }
    }
    if (!availableFound) {
        printf("[Info] No plates available for request at this time.\n");
        return 0;
    }

    int pid;
    while (1) {
        printf("\nEnter plate ID to request (0 to cancel): ");
        char idStr[20];
        fgets(idStr, sizeof(idStr), stdin);
        idStr[strcspn(idStr, "\n")] = 0;

        if (strlen(idStr) == 0) {
            printf("[Warning] Please enter a plate ID or 0 to cancel.\n");
            continue;
        }
        if (sscanf(idStr, "%d", &pid) != 1) {
            printf("[Error] Invalid input! Please enter a valid number.\n");
            continue;
        }
        break;
    }

    if (pid == 0) { printf("Request cancelled.\n"); return 0; }

    int idx = -1;
    for (int i = 0; i < plateCount; i++) {
        if (plates[i].id == pid && strcmp(plates[i].status, "Available") == 0) { idx = i; break; }
    }
    if (idx == -1) { printf("[Error] Plate #%d not available or invalid ID.\n", pid); return 0; }
    if (strcmp(plates[idx].donor, requester) == 0) {
        printf("[Error] You cannot request your own plate!\n");
        return 0;
    }

    Request r;
    r.id = (*reqCount > 0) ? requests[*reqCount - 1].id + 1 : 1;
    r.plateId = pid;
    strcpy(r.requester, requester);
    strcpy(r.status, "Pending");
    getCurrentTimestamp(r.timestamp, sizeof(r.timestamp));
    strcpy(plates[idx].status, "Reserved");
    requests[*reqCount] = r; (*reqCount)++;
    saveRequests(requests, *reqCount);
    savePlates(plates, plateCount);

    printf("[Success] Request sent! (Request ID: %d)\n", r.id);

    char notifMsg[MAX_MSG];
    snprintf(notifMsg, sizeof(notifMsg), "%s requested your plate '%s' (Req #%d)",
        requester, plates[idx].foodName, r.id);
    addNotification(plates[idx].donor, notifMsg);
    return 1;
}

void viewRequestsForMyPlates(const Request *requests, int reqCount,
                              const Plate *plates, int plateCount, const char *donor) {
    printf("\n--- Requests for Your Plates ---\n");
    int f = 0;
    for (int i = reqCount - 1; i >= 0; i--) {
        for (int j = 0; j < plateCount; j++) {
            if (plates[j].id == requests[i].plateId &&
                strcmp(plates[j].donor, donor) == 0) {
                printf("Req#%d | Plate: %s | From: %s | Status: %s | Time: %s\n",
                    requests[i].id, plates[j].foodName,
                    requests[i].requester, requests[i].status,
                    requests[i].timestamp);
                f++;
            }
        }
    }
    if (!f) printf("No requests for your plates.\n");
}

void filterMyRequests(const Request *requests, int reqCount,
                       const Plate *plates, int plateCount, const char *donor) {
    int choice;
    while (1) {
        system(CLEAR_SCREEN);
        printf("\n");
        printCenteredLine('=', 36);
        printCentered("FILTER REQUESTS");
        printCenteredLine('=', 36);
        printf("                    1. Show Pending Only\n");
        printf("                    2. Show Accepted Only\n");
        printf("                    3. Show Declined Only\n");
        printf("                    4. Show All (Most Recent First)\n");
        printf("                    5. Back\n");
        printCenteredLine('=', 36);
        printf("                    Choice: ");
        scanf("%d", &choice); getchar();

        if (choice == 5) return;

        printf("\n--- Filtered Requests ---\n");
        int f = 0;
        for (int i = reqCount - 1; i >= 0; i--) {
            for (int j = 0; j < plateCount; j++) {
                if (plates[j].id == requests[i].plateId &&
                    strcmp(plates[j].donor, donor) == 0) {
                    int show = 0;
                    if (choice == 1 && strcmp(requests[i].status, "Pending") == 0) show = 1;
                    else if (choice == 2 && strcmp(requests[i].status, "Accepted") == 0) show = 1;
                    else if (choice == 3 && strcmp(requests[i].status, "Declined") == 0) show = 1;
                    else if (choice == 4) show = 1;

                    if (show) {
                        printf("Req#%d | %s | %s | %s | %s\n", requests[i].id, plates[j].foodName,
                            requests[i].requester, requests[i].status, requests[i].timestamp);
                        f++;
                    }
                }
            }
        }
        if (!f) printf("No matching requests.\n");
        printf("\n                    Press Enter to continue..."); getchar();
    }
}

int acceptOrDeclineRequest(int reqId, Request *requests, int reqCount,
                            Plate *plates, int plateCount) {
    int idx = -1;
    for (int i = 0; i < reqCount; i++) {
        if (requests[i].id == reqId) { idx = i; break; }
    }
    if (idx == -1 || strcmp(requests[idx].status, "Pending") != 0) {
        printf("[Error] Request not found or already processed.\n");
        return 0;
    }

    printf("\nRequest #%d: %s wants your plate (Plate ID: %d)\n",
        reqId, requests[idx].requester, requests[idx].plateId);
    printf("Accept or Decline? (a/d, any other key to cancel): ");
    char confirm;
    scanf(" %c", &confirm); getchar();

    if (confirm == 'a' || confirm == 'A') {
        printf("Are you sure you want to ACCEPT? (y/n): ");
        char sure; scanf(" %c", &sure); getchar();
        if (sure != 'y' && sure != 'Y') { printf("Cancelled.\n"); return 0; }

        strcpy(requests[idx].status, "Accepted");
        for (int j = 0; j < plateCount; j++)
            if (plates[j].id == requests[idx].plateId) { strcpy(plates[j].status, "Donated"); break; }
        saveRequests(requests, reqCount); savePlates(plates, plateCount);
        printf("[Success] Request #%d ACCEPTED! Plate marked as Donated.\n", reqId);

        char notifMsg[MAX_MSG];
        snprintf(notifMsg, sizeof(notifMsg), "Your request #%d was ACCEPTED!", reqId);
        addNotification(requests[idx].requester, notifMsg);
        return 1;
    } else if (confirm == 'd' || confirm == 'D') {
        printf("Are you sure you want to DECLINE? (y/n): ");
        char sure; scanf(" %c", &sure); getchar();
        if (sure != 'y' && sure != 'Y') { printf("Cancelled.\n"); return 0; }

        strcpy(requests[idx].status, "Declined");
        for (int j = 0; j < plateCount; j++)
            if (plates[j].id == requests[idx].plateId) { strcpy(plates[j].status, "Available"); break; }
        saveRequests(requests, reqCount); savePlates(plates, plateCount);
        printf("[Success] Request #%d DECLINED! Plate is now Available.\n", reqId);

        char notifMsg[MAX_MSG];
        snprintf(notifMsg, sizeof(notifMsg), "Your request #%d was DECLINED.", reqId);
        addNotification(requests[idx].requester, notifMsg);
        return 1;
    }
    printf("Cancelled.\n");
    return 0;
}

void viewTransactionHistory(const Request *requests, int reqCount, const char *username) {
    printf("\n--- Transaction History for %s (Most Recent First) ---\n", username);
    int f = 0;
    for (int i = reqCount - 1; i >= 0; i--) {
        if (strcmp(requests[i].requester, username) == 0) {
            printf("Req#%d | Plate ID: %d | Status: %s | Time: %s\n",
                requests[i].id, requests[i].plateId,
                requests[i].status, requests[i].timestamp);
            f++;
        }
    }
    if (!f) printf("No transactions found.\n");
    else printf("Total: %d request(s)\n", f);
}

void requestMenu(const char *loggedInUser) {
    int choice, reqId;
    while (1) {
        system(CLEAR_SCREEN);
        printf("\n");
        printCenteredLine('=', 36);
        printCentered("REQUEST MANAGEMENT");
        printCenteredLine('=', 36);
        printf("                    1. Make New Request\n");
        printf("                    2. View Requests for My Plates\n");
        printf("                    3. Filter My Requests\n");
        printf("                    4. Accept/Decline Request\n");
        printf("                    5. Transaction History\n");
        printf("                    6. Back to Main Menu\n");
        printCenteredLine('=', 36);
        printf("                    Choice: ");
        scanf("%d", &choice); getchar();

        switch (choice) {
            case 1:
                system(CLEAR_SCREEN);
                printf("\n--- Make New Request ---\n");
                loadPlates(plates, &plateCount); loadRequests(requests, &requestCount);
                createRequest(requests, &requestCount, plates, plateCount, loggedInUser);
                printf("\n                    Press Enter to continue..."); getchar();
                break;
            case 2:
                system(CLEAR_SCREEN);
                loadPlates(plates, &plateCount); loadRequests(requests, &requestCount);
                viewRequestsForMyPlates(requests, requestCount, plates, plateCount, loggedInUser);
                printf("\n                    Press Enter to continue..."); getchar();
                break;
            case 3:
                loadPlates(plates, &plateCount); loadRequests(requests, &requestCount);
                filterMyRequests(requests, requestCount, plates, plateCount, loggedInUser);
                break;
            case 4:
                system(CLEAR_SCREEN);
                printf("\n--- Accept/Decline Request ---\n");
                loadPlates(plates, &plateCount); loadRequests(requests, &requestCount);
                viewRequestsForMyPlates(requests, requestCount, plates, plateCount, loggedInUser);
                printf("\nEnter Request ID (0 to cancel): ");
                if (scanf("%d", &reqId) != 1) {
                    printf("[Error] Invalid input!\n"); while (getchar() != '\n');
                } else {
                    getchar();
                    if (reqId != 0) acceptOrDeclineRequest(reqId, requests, requestCount, plates, plateCount);
                    else printf("Cancelled.\n");
                }
                printf("\n                    Press Enter to continue..."); getchar();
                break;
            case 5:
                system(CLEAR_SCREEN);
                loadRequests(requests, &requestCount);
                viewTransactionHistory(requests, requestCount, loggedInUser);
                printf("\n                    Press Enter to continue..."); getchar();
                break;
            case 6: return;
            default: printf("\n"); printCentered("Invalid choice.");
        }
    }
}