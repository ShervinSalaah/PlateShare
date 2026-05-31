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
    displayAllPlates(plates, plateCount);
    int pid;
    printf("Enter plate ID: "); scanf("%d", &pid); getchar();
    int idx = -1;
    for (int i = 0; i < plateCount; i++)
        if (plates[i].id == pid && strcmp(plates[i].status, "Available") == 0) { idx = i; break; }
    if (idx == -1) { printf("Not available.\n"); return 0; }
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
    printf("Request sent (ID: %d).\n", r.id);
    char notifMsg[MAX_MSG];
    snprintf(notifMsg, sizeof(notifMsg), "%s requested your plate: %s", requester, plates[idx].foodName);
    addNotification(plates[idx].donor, notifMsg);
    return 1;
}

void viewRequestsForMyPlates(const Request *requests, int reqCount, const Plate *plates, int plateCount, const char *donor) {
    printf("\n--- Requests for Your Plates ---\n");
    int f = 0;
    for (int i = 0; i < reqCount; i++)
        for (int j = 0; j < plateCount; j++)
            if (plates[j].id == requests[i].plateId && strcmp(plates[j].donor, donor) == 0) {
                printf("Req#%d | %s | by %s | %s | %s\n", requests[i].id, plates[j].foodName,
                    requests[i].requester, requests[i].status, requests[i].timestamp);
                f++;
            }
    if (!f) printf("No requests.\n");
}

int acceptRequest(int reqId, Request *requests, int reqCount, Plate *plates, int plateCount) {
    for (int i = 0; i < reqCount; i++) {
        if (requests[i].id == reqId && strcmp(requests[i].status, "Pending") == 0) {
            strcpy(requests[i].status, "Accepted");
            for (int j = 0; j < plateCount; j++)
                if (plates[j].id == requests[i].plateId) { strcpy(plates[j].status, "Donated"); break; }
            saveRequests(requests, reqCount); savePlates(plates, plateCount);
            printf("Accepted.\n");
            char notifMsg[MAX_MSG];
            snprintf(notifMsg, sizeof(notifMsg), "Your request #%d was ACCEPTED!", reqId);
            addNotification(requests[i].requester, notifMsg);
            return 1;
        }
    }
    printf("Not found or already processed.\n"); return 0;
}

int declineRequest(int reqId, Request *requests, int reqCount, Plate *plates, int plateCount) {
    for (int i = 0; i < reqCount; i++) {
        if (requests[i].id == reqId && strcmp(requests[i].status, "Pending") == 0) {
            strcpy(requests[i].status, "Declined");
            for (int j = 0; j < plateCount; j++)
                if (plates[j].id == requests[i].plateId) { strcpy(plates[j].status, "Available"); break; }
            saveRequests(requests, reqCount); savePlates(plates, plateCount);
            printf("Declined.\n");
            char notifMsg[MAX_MSG];
            snprintf(notifMsg, sizeof(notifMsg), "Your request #%d was DECLINED.", reqId);
            addNotification(requests[i].requester, notifMsg);
            return 1;
        }
    }
    printf("Not found or already processed.\n"); return 0;
}

void viewTransactionHistory(const Request *requests, int reqCount, const char *username) {
    printf("\n--- History for %s ---\n", username);
    int f = 0;
    for (int i = 0; i < reqCount; i++)
        if (strcmp(requests[i].requester, username) == 0) {
            printf("Req#%d | Plate#%d | %s | %s\n", requests[i].id, requests[i].plateId,
                requests[i].status, requests[i].timestamp);
            f++;
        }
    if (!f) printf("No transactions.\n");
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
        printf("                    3. Accept Request\n");
        printf("                    4. Decline Request\n");
        printf("                    5. Transaction History\n");
        printf("                    6. Back to Main Menu\n");
        printCenteredLine('=', 36);
        printf("                    Choice: ");
        scanf("%d", &choice); getchar();
        switch (choice) {
            case 1:
                system(CLEAR_SCREEN); printf("\n--- Make New Request ---\n");
                loadPlates(plates, &plateCount); loadRequests(requests, &requestCount);
                createRequest(requests, &requestCount, plates, plateCount, loggedInUser);
                printf("\n                    Press Enter to continue..."); getchar(); break;
            case 2:
                system(CLEAR_SCREEN);
                loadPlates(plates, &plateCount); loadRequests(requests, &requestCount);
                viewRequestsForMyPlates(requests, requestCount, plates, plateCount, loggedInUser);
                printf("\n                    Press Enter to continue..."); getchar(); break;
            case 3:
                system(CLEAR_SCREEN); printf("\n--- Accept Request ---\n");
                loadPlates(plates, &plateCount); loadRequests(requests, &requestCount);
                viewRequestsForMyPlates(requests, requestCount, plates, plateCount, loggedInUser);
                printf("Request ID to accept: "); scanf("%d", &reqId); getchar();
                acceptRequest(reqId, requests, requestCount, plates, plateCount);
                printf("\n                    Press Enter to continue..."); getchar(); break;
            case 4:
                system(CLEAR_SCREEN); printf("\n--- Decline Request ---\n");
                loadPlates(plates, &plateCount); loadRequests(requests, &requestCount);
                viewRequestsForMyPlates(requests, requestCount, plates, plateCount, loggedInUser);
                printf("Request ID to decline: "); scanf("%d", &reqId); getchar();
                declineRequest(reqId, requests, requestCount, plates, plateCount);
                printf("\n                    Press Enter to continue..."); getchar(); break;
            case 5:
                system(CLEAR_SCREEN); loadRequests(requests, &requestCount);
                viewTransactionHistory(requests, requestCount, loggedInUser);
                printf("\n                    Press Enter to continue..."); getchar(); break;
            case 6: return;
            default: printf("\n"); printCentered("Invalid choice.");
        }
    }
}