#include "config.h"
#include "core.h"
#include "user.h"
#include "plate.h"
#include "request.h"
#include "chat.h"

Config appConfig;
User users[MAX_USERS];       int userCount = 0;
Plate plates[MAX_PLATES];    int plateCount = 0;
Request requests[MAX_REQUESTS]; int requestCount = 0;

int main() {
    initSystem(&appConfig);
    loadUsers(users, &userCount);
    loadPlates(plates, &plateCount);
    loadRequests(requests, &requestCount);

    char loggedInUser[30] = "";
    int loggedIn = 0;
    int choice, subChoice;
    char input[MAX_LINE];
    char chatPath[MAX_PATH];
    char recipient[30];

    while (1) {
        choice = displayMainMenu();
        switch (choice) {
            case 1:
                printf("\n1. Register\n2. Login\n3. View All Users\n4. Update Profile\n5. Logout\nChoice: ");
                scanf("%d", &subChoice);
                getchar();
                switch (subChoice) {
                    case 1: registerUser(users, &userCount); break;
                    case 2: loggedIn = loginUser(users, userCount, loggedInUser); break;
                    case 3: displayAllUsers(users, userCount); break;
                    case 4:
                        if (loggedIn) updateUserProfile(users, userCount, loggedInUser);
                        else printf("Please login first.\n");
                        break;
                    case 5: loggedIn = 0; strcpy(loggedInUser, ""); printf("Logged out.\n"); break;
                    default: printf("Invalid choice.\n");
                }
                break;

            case 2:
                if (!loggedIn) { printf("Please login first.\n"); break; }
                printf("\n1. Add Plate\n2. View All Plates\n3. Search Plate\n4. Sort by Expiry\nChoice: ");
                scanf("%d", &subChoice);
                getchar();
                switch (subChoice) {
                    case 1: addPlate(plates, &plateCount, loggedInUser); break;
                    case 2: displayAllPlates(plates, plateCount); break;
                    case 3:
                        printf("Search term: ");
                        fgets(input, sizeof(input), stdin);
                        input[strcspn(input, "\n")] = 0;
                        searchPlateByName(plates, plateCount, input);
                        break;
                    case 4: sortPlatesByExpiry(plates, plateCount); displayAllPlates(plates, plateCount); break;
                    default: printf("Invalid choice.\n");
                }
                break;

            case 3:
                if (!loggedIn) { printf("Please login first.\n"); break; }
                printf("\n1. Make Request\n2. View Requests for My Plates\n3. Accept Request\n4. Decline Request\n5. Transaction History\nChoice: ");
                scanf("%d", &subChoice);
                getchar();
                int reqId;
                switch (subChoice) {
                    case 1: createRequest(requests, &requestCount, plates, plateCount, loggedInUser); break;
                    case 2: viewRequestsForMyPlates(requests, requestCount, plates, plateCount, loggedInUser); break;
                    case 3:
                        printf("Request ID to accept: ");
                        scanf("%d", &reqId); getchar();
                        acceptRequest(reqId, requests, requestCount, plates, plateCount);
                        break;
                    case 4:
                        printf("Request ID to decline: ");
                        scanf("%d", &reqId); getchar();
                        declineRequest(reqId, requests, requestCount, plates, plateCount);
                        break;
                    case 5: viewTransactionHistory(requests, requestCount, loggedInUser); break;
                    default: printf("Invalid choice.\n");
                }
                break;

            case 4:
                if (!loggedIn) { printf("Please login first.\n"); break; }
                snprintf(chatPath, MAX_PATH, "%schat.txt", appConfig.dataFolder);
                printf("\n1. Group Chat (Live)\n2. Send Direct Message\n3. Inbox (Private Messages)\n4. Notifications\n5. Clear Notifications\nChoice: ");
                scanf("%d", &subChoice);
                getchar();
                switch (subChoice) {
                    case 1:
                        viewGroupChat(chatPath, loggedInUser);
                        break;
                    case 2:
                        printf("Send to (username): ");
                        fgets(recipient, sizeof(recipient), stdin);
                        recipient[strcspn(recipient, "\n")] = 0;
                        printf("Message: ");
                        fgets(input, sizeof(input), stdin);
                        input[strcspn(input, "\n")] = 0;
                        sendDirectMessage(loggedInUser, recipient, input, chatPath);
                        break;
                    case 3:
                        viewInbox(chatPath, loggedInUser);
                        break;
                    case 4:
                        viewNotifications(loggedInUser);
                        break;
                    case 5:
                        clearNotifications(loggedInUser);
                        break;
                    default:
                        printf("Invalid choice.\n");
                }
                break;

            case 5:
                printf("\nCurrent Settings:\n");
                printf("Data Folder: %s\n", appConfig.dataFolder);
                printf("Max Users: %d\n", appConfig.maxUsers);
                printf("Max Plates: %d\n", appConfig.maxPlates);
                printf("Max Requests: %d\n", appConfig.maxRequests);
                printf("Change data folder? (y/n): ");
                char yn;
                scanf(" %c", &yn);
                getchar();
                if (yn == 'y' || yn == 'Y') {
                    printf("New path: ");
                    fgets(input, sizeof(input), stdin);
                    input[strcspn(input, "\n")] = 0;
                    strcpy(appConfig.dataFolder, input);
                    saveConfig(&appConfig);
                    printf("Updated. Restart may be required.\n");
                }
                break;

            case 6:
                printf("Goodbye!\n");
                return 0;

            default:
                printf("Invalid option.\n");
        }
    }
}