/**
 * @file request.c
 * @brief Food request and transaction management implementation
 * @author 254189P SALIYA J.F
 * 
 * Handles all request-related operations:
 * - Requesting available food from donors
 * - Viewing requests made on your food
 * - Filtering requests by status (Pending/Accepted/Declined)
 * - Accepting or declining requests with confirmation
 * - Viewing transaction history (most recent first)
 * - Sending automatic notifications to both parties
 */

#include "request.h"
#include "chat.h"
#include "core.h"

/**
 * @brief Loads all requests from requests.csv into memory
 */
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

/**
 * @brief Saves all requests to requests.csv
 */
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

/**
 * @brief Creates a new request for an available food plate
 * 
 * Shows only available plates, validates the plate ID input,
 * prevents requesting your own food, and sends a notification
 * to the plate owner.
 * 
 * @return 1 if request created, 0 if cancelled or failed
 */
int createRequest(Request *requests, int *reqCount, Plate *plates, int plateCount, const char *requester) {
    printf("\n--- Food Available to Request ---\n");
    int availableFound = 0;
    for (int i = 0; i < plateCount; i++) {
        if (strcmp(plates[i].status, "Available") == 0) {
            printf("  ID: %d | %s | From: %s | Portions: %d | Expires: %s | Pickup: %s\n",
                plates[i].id, plates[i].foodName, plates[i].donor,
                plates[i].quantity, plates[i].expiryDate, plates[i].pickupOption);
            printf("    Location: %s\n", plates[i].address);
            availableFound++;
        }
    }
    if (!availableFound) {
        printf("\n  No food available right now. Check back later!\n");
        pauseScreen("");
        return 0;
    }

    int pid;
    while (1) {
        printf("\n  Enter the Food ID you want to request (0 to cancel): ");
        char idStr[20];
        fgets(idStr, sizeof(idStr), stdin);
        idStr[strcspn(idStr, "\n")] = 0;

        if (strlen(idStr) == 0) {
            printf("  Please enter a number. Cannot be empty.\n");
            continue;
        }
        if (sscanf(idStr, "%d", &pid) != 1) {
            printf("  That's not a valid number. Please try again.\n");
            continue;
        }
        break;
    }

    if (pid == 0) { printf("  Request cancelled.\n"); pauseScreen(""); return 0; }

    /* Find the plate */
    int idx = -1;
    for (int i = 0; i < plateCount; i++) {
        if (plates[i].id == pid && strcmp(plates[i].status, "Available") == 0) {
            idx = i;
            break;
        }
    }
    if (idx == -1) {
        printf("  Food #%d is not available or doesn't exist.\n", pid);
        pauseScreen("");
        return 0;
    }

    /* Prevent requesting your own food */
    if (strcmp(plates[idx].donor, requester) == 0) {
        printf("  You cannot request your own food!\n");
        pauseScreen("");
        return 0;
    }

    /* Create the request */
    Request r;
    r.id = (*reqCount > 0) ? requests[*reqCount - 1].id + 1 : 1;
    r.plateId = pid;
    strcpy(r.requester, requester);
    strcpy(r.status, "Pending");
    getCurrentTimestamp(r.timestamp, sizeof(r.timestamp));

    /* Mark plate as Reserved */
    strcpy(plates[idx].status, "Reserved");

    /* Save */
    requests[*reqCount] = r;
    (*reqCount)++;
    saveRequests(requests, *reqCount);
    savePlates(plates, plateCount);

    printf("\n  Request sent successfully! (Request #%d)\n", r.id);
    printf("  You asked for: %s from %s\n", plates[idx].foodName, plates[idx].donor);
    printf("  Pickup: %s | Location: %s\n", plates[idx].pickupOption, plates[idx].address);

    /* Notify the food owner */
    char notifMsg[MAX_MSG];
    snprintf(notifMsg, sizeof(notifMsg), "%s wants your food '%s' (Request #%d)",
        requester, plates[idx].foodName, r.id);
    addNotification(plates[idx].donor, notifMsg);

    pauseScreen("");
    return 1;
}

/**
 * @brief Shows all requests for food owned by the logged-in donor
 * 
 * Displays requests in reverse order (most recent first).
 */
void viewRequestsForMyPlates(const Request *requests, int reqCount,
                              const Plate *plates, int plateCount, const char *donor) {
    printf("\n--- Requests for Your Food (Newest First) ---\n");
    int f = 0;
    for (int i = reqCount - 1; i >= 0; i--) {
        for (int j = 0; j < plateCount; j++) {
            if (plates[j].id == requests[i].plateId &&
                strcmp(plates[j].donor, donor) == 0) {
                printf("  Req#%d | Food: %s | From: %s | Status: %s | Time: %s\n",
                    requests[i].id, plates[j].foodName,
                    requests[i].requester, requests[i].status,
                    requests[i].timestamp);
                f++;
            }
        }
    }
    if (!f) printf("  No requests for your food yet.\n");
}

/**
 * @brief Filters requests by status
 * 
 * Allows viewing only Pending, Accepted, or Declined requests
 * for the donor's plates.
 */
void filterMyRequests(const Request *requests, int reqCount,
                       const Plate *plates, int plateCount, const char *donor) {
    int choice;
    while (1) {
        system(CLEAR_SCREEN);
        printf("\n");
        printCenteredLine('=', 40);
        printCentered("FILTER REQUESTS");
        printCenteredLine('=', 40);
        printf("                    1. Show Waiting (Pending)\n");
        printf("                    2. Show Accepted\n");
        printf("                    3. Show Declined\n");
        printf("                    4. Show All (Newest First)\n");
        printf("                    5. Go Back\n");
        printCenteredLine('=', 40);
        printf("                    Your choice: ");
        scanf("%d", &choice); getchar();

        if (choice == 5) return;

        printf("\n  --- Filtered Results ---\n");
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
                        printf("  Req#%d | %s | %s | %s | %s\n",
                            requests[i].id, plates[j].foodName,
                            requests[i].requester, requests[i].status,
                            requests[i].timestamp);
                        f++;
                    }
                }
            }
        }
        if (!f) printf("  No matching requests.\n");
        pauseScreen("");
    }
}

/**
 * @brief Accepts or declines a pending request with confirmation
 * 
 * Shows the request details and asks the donor to choose
 * Accept (a) or Decline (d). Requires a second confirmation
 * before making the change.
 */
int acceptOrDeclineRequest(int reqId, Request *requests, int reqCount,
                            Plate *plates, int plateCount) {
    /* Find the request */
    int idx = -1;
    for (int i = 0; i < reqCount; i++) {
        if (requests[i].id == reqId) { idx = i; break; }
    }
    if (idx == -1 || strcmp(requests[idx].status, "Pending") != 0) {
        printf("\n  Request not found or already handled.\n");
        pauseScreen("");
        return 0;
    }

    /* Show request details */
    printf("\n  Request #%d: %s wants your food (Plate ID: %d)\n",
        reqId, requests[idx].requester, requests[idx].plateId);
    printf("  Accept (a) or Decline (d)? Any other key to cancel: ");
    char confirm;
    scanf(" %c", &confirm); getchar();

    if (confirm == 'a' || confirm == 'A') {
        /* Accept - ask confirmation */
        printf("  Are you sure you want to ACCEPT? (y/n): ");
        char sure; scanf(" %c", &sure); getchar();
        if (sure != 'y' && sure != 'Y') {
            printf("  Cancelled.\n"); pauseScreen(""); return 0;
        }

        strcpy(requests[idx].status, "Accepted");
        for (int j = 0; j < plateCount; j++) {
            if (plates[j].id == requests[idx].plateId) {
                strcpy(plates[j].status, "Donated");
                break;
            }
        }
        saveRequests(requests, reqCount);
        savePlates(plates, plateCount);
        printf("\n  Request #%d ACCEPTED! The food is now marked as Given.\n", reqId);

        /* Notify requester */
        char notifMsg[MAX_MSG];
        snprintf(notifMsg, sizeof(notifMsg), "Great news! Your request #%d was ACCEPTED!", reqId);
        addNotification(requests[idx].requester, notifMsg);
        pauseScreen("");
        return 1;

    } else if (confirm == 'd' || confirm == 'D') {
        /* Decline - ask confirmation */
        printf("  Are you sure you want to DECLINE? (y/n): ");
        char sure; scanf(" %c", &sure); getchar();
        if (sure != 'y' && sure != 'Y') {
            printf("  Cancelled.\n"); pauseScreen(""); return 0;
        }

        strcpy(requests[idx].status, "Declined");
        for (int j = 0; j < plateCount; j++) {
            if (plates[j].id == requests[idx].plateId) {
                strcpy(plates[j].status, "Available");
                break;
            }
        }
        saveRequests(requests, reqCount);
        savePlates(plates, plateCount);
        printf("\n  Request #%d DECLINED. The food is available again.\n", reqId);

        /* Notify requester */
        char notifMsg[MAX_MSG];
        snprintf(notifMsg, sizeof(notifMsg), "Your request #%d was DECLINED.", reqId);
        addNotification(requests[idx].requester, notifMsg);
        pauseScreen("");
        return 1;
    }

    printf("  Cancelled.\n");
    pauseScreen("");
    return 0;
}

/**
 * @brief Shows all requests made by the logged-in user
 * 
 * Displays in reverse order (most recent first) with status.
 */
void viewTransactionHistory(const Request *requests, int reqCount, const char *username) {
    printf("\n--- Your Request History (Newest First) ---\n");
    int f = 0;
    for (int i = reqCount - 1; i >= 0; i--) {
        if (strcmp(requests[i].requester, username) == 0) {
            printf("  Req#%d | Food ID: %d | Status: %s | Time: %s\n",
                requests[i].id, requests[i].plateId,
                requests[i].status, requests[i].timestamp);
            f++;
        }
    }
    if (!f) printf("  You haven't made any requests yet.\n");
    else printf("  Total: %d request(s)\n", f);
}

/**
 * @brief Food request sub-menu
 */
void requestMenu(const char *loggedInUser) {
    int choice, reqId;
    while (1) {
        system(CLEAR_SCREEN);
        printf("\n");
        printCenteredLine('=', 40);
        printCentered("REQUEST FOOD");
        printCenteredLine('=', 40);
        printf("                    1. Request Available Food\n");
        printf("                    2. See Requests for My Food\n");
        printf("                    3. Filter My Requests\n");
        printf("                    4. Accept or Decline\n");
        printf("                    5. My Request History\n");
        printf("                    6. Go Back\n");
        printCenteredLine('=', 40);
        printf("                    Your choice: ");
        scanf("%d", &choice); getchar();

        switch (choice) {
            case 1:
                system(CLEAR_SCREEN);
                loadPlates(plates, &plateCount);
                loadRequests(requests, &requestCount);
                createRequest(requests, &requestCount, plates, plateCount, loggedInUser);
                break;
            case 2:
                system(CLEAR_SCREEN);
                loadPlates(plates, &plateCount);
                loadRequests(requests, &requestCount);
                viewRequestsForMyPlates(requests, requestCount, plates, plateCount, loggedInUser);
                pauseScreen("");
                break;
            case 3:
                loadPlates(plates, &plateCount);
                loadRequests(requests, &requestCount);
                filterMyRequests(requests, requestCount, plates, plateCount, loggedInUser);
                break;
            case 4:
                system(CLEAR_SCREEN);
                printf("\n--- Accept or Decline Request ---\n");
                loadPlates(plates, &plateCount);
                loadRequests(requests, &requestCount);
                viewRequestsForMyPlates(requests, requestCount, plates, plateCount, loggedInUser);
                printf("\n  Enter Request ID (0 to cancel): ");
                if (scanf("%d", &reqId) != 1) {
                    printf("  Invalid input!\n");
                    while (getchar() != '\n');
                    pauseScreen("");
                } else {
                    getchar();
                    if (reqId != 0) acceptOrDeclineRequest(reqId, requests, requestCount, plates, plateCount);
                    else printf("  Cancelled.\n"); pauseScreen("");
                }
                break;
            case 5:
                system(CLEAR_SCREEN);
                loadRequests(requests, &requestCount);
                viewTransactionHistory(requests, requestCount, loggedInUser);
                pauseScreen("");
                break;
            case 6: return;
            default:
                printf("\n"); printCentered("Wrong choice!"); pauseScreen("");
        }
    }
}