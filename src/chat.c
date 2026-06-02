/**
 * @file chat.c
 * @brief Implementation of chat and notification system
 * @author Member 5 - Salaah
 */

#include "chat.h"
#include "core.h"
#include <ctype.h>

/* ========== Private Helper Functions ========== */

static void getNotificationPath(char *path, size_t size, const char *username) {
    snprintf(path, size, "%snotifications_%s.txt", appConfig.dataFolder, username);
}

static int isGroupMessage(const char *line) {
    return strstr(line, " -> ") == NULL;
}

static int isMessageForUser(const char *line, const char *username) {
    char copy[MAX_LINE];
    strcpy(copy, line);
    char *arrow = strstr(copy, " -> ");
    if (arrow == NULL) return 0;
    char *recipStart = arrow + 4;
    char *colon = strstr(recipStart, ": ");
    if (colon == NULL) return 0;
    *colon = '\0';
    return strcmp(recipStart, username) == 0;
}

static int isMessageFromUser(const char *line, const char *username) {
    char copy[MAX_LINE];
    strcpy(copy, line);
    char *bracket = strchr(copy, ']');
    if (bracket == NULL) return 0;
    char *space = strchr(bracket + 2, ' ');
    if (space == NULL) return 0;
    *space = '\0';
    return strcmp(bracket + 2, username) == 0;
}

/**
 * @brief Reads all lines from a file into an array of strings for sorting
 */
static int readAllLines(const char *filePath, char lines[][MAX_LINE], int maxLines) {
    FILE *fp = fopen(filePath, "r");
    if (fp == NULL) return 0;
    int count = 0;
    while (fgets(lines[count], MAX_LINE, fp) && count < maxLines) {
        lines[count][strcspn(lines[count], "\n")] = 0;
        count++;
    }
    fclose(fp);
    return count;
}

/* ========== Public Functions ========== */

int userExists(const char *username) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, username) == 0) return 1;
    }
    return 0;
}

void viewGroupChat(const char *chatFilePath, const char *currentUser) {
    char line[MAX_LINE];
    long lastSize = 0;
    FILE *fp = fopen(chatFilePath, "r");
    if (fp) { fseek(fp, 0, SEEK_END); lastSize = ftell(fp); fclose(fp); }

    printf("\n");
    printCenteredLine('=', 36);
    printCentered("GROUP CHAT");
    printCentered("/q quit, /dm user msg for direct");
    printCenteredLine('=', 36);
    printf("\n");

    fp = fopen(chatFilePath, "r");
    if (fp) {
        while (fgets(line, sizeof(line), fp)) {
            if (isGroupMessage(line)) printf("%s", line);
        }
        fclose(fp);
    }

    while (1) {
        SLEEP(1);
        fp = fopen(chatFilePath, "r");
        if (fp) {
            fseek(fp, 0, SEEK_END);
            long cur = ftell(fp);
            if (cur > lastSize) {
                fseek(fp, lastSize, SEEK_SET);
                while (fgets(line, sizeof(line), fp)) {
                    if (isGroupMessage(line)) printf("%s", line);
                }
                lastSize = cur;
            }
            fclose(fp);
        }
        printf("> ");
        char input[MAX_MSG];
        if (fgets(input, sizeof(input), stdin) == NULL) continue;
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) continue;
        if (strcmp(input, "/q") == 0) {
            printf("Exiting group chat.\n");
            break;
        }
        if (strncmp(input, "/dm ", 4) == 0) {
            char *rest = input + 4;
            char *firstSpace = strchr(rest, ' ');
            if (firstSpace) {
                *firstSpace = '\0';
                if (!userExists(rest)) {
                    printf("User '%s' does not exist.\n", rest);
                } else {
                    sendDirectMessage(currentUser, rest, firstSpace + 1, chatFilePath);
                }
            } else {
                printf("Usage: /dm username message\n");
            }
        } else {
            FILE *fpApp = fopen(chatFilePath, "a");
            if (fpApp) {
                char ts[20];
                getCurrentTimestamp(ts, sizeof(ts));
                fprintf(fpApp, "[%s] %s: %s\n", ts, currentUser, input);
                fclose(fpApp);
            }
        }
    }
}

void sendDirectMessage(const char *sender, const char *recipient, const char *msg, const char *chatFilePath) {
    if (strlen(recipient) == 0) {
        printf("[Cancelled] No recipient specified. Message not sent.\n");
        return;
    }
    if (!userExists(recipient)) {
        printf("[Error] User '%s' does not exist!\n", recipient);
        printf("  [1] Try a different username\n");
        printf("  [2] Go back to menu\n");
        printf("  Choice: ");
        char c; scanf(" %c", &c); getchar();
        return;
    }
    if (strlen(msg) == 0) {
        printf("[Cancelled] Empty message. Not sent.\n");
        return;
    }

    FILE *fp = fopen(chatFilePath, "a");
    if (fp == NULL) { printf("Error opening chat file.\n"); return; }
    char ts[20];
    getCurrentTimestamp(ts, sizeof(ts));
    fprintf(fp, "[%s] %s -> %s: %s\n", ts, sender, recipient, msg);
    fclose(fp);
    printf("[Success] Direct message sent to %s.\n", recipient);

    char notifMsg[MAX_MSG + 60];
    snprintf(notifMsg, sizeof(notifMsg), "New message from %s: %.100s", sender, msg);
    addNotification(recipient, notifMsg);
}

void viewInbox(const char *chatFilePath, const char *currentUser) {
    char allLines[1000][MAX_LINE];
    int totalLines = readAllLines(chatFilePath, allLines, 1000);

    printf("\n");
    printCenteredLine('=', 36);
    printCentered("INBOX (Received Messages)");
    printCenteredLine('=', 36);
    printf("\n");

    int hasMsgs = 0;
    /* Show most recent first (iterate backwards) */
    for (int i = totalLines - 1; i >= 0; i--) {
        if (isMessageForUser(allLines[i], currentUser) && !isGroupMessage(allLines[i])) {
            printf("%s\n", allLines[i]);
            hasMsgs = 1;
        }
    }
    if (!hasMsgs) printf("No received messages.\n");

    printf("\n                    Press Enter to continue..."); getchar();
}

void viewSentMessages(const char *chatFilePath, const char *currentUser) {
    char allLines[1000][MAX_LINE];
    int totalLines = readAllLines(chatFilePath, allLines, 1000);

    printf("\n");
    printCenteredLine('=', 36);
    printCentered("SENT (Messages You Sent)");
    printCenteredLine('=', 36);
    printf("\n");

    int hasMsgs = 0;
    /* Show most recent first (iterate backwards) */
    for (int i = totalLines - 1; i >= 0; i--) {
        if (isMessageFromUser(allLines[i], currentUser) && !isGroupMessage(allLines[i])) {
            printf("%s\n", allLines[i]);
            hasMsgs = 1;
        }
    }
    if (!hasMsgs) printf("No sent messages.\n");

    printf("\n                    Press Enter to continue..."); getchar();
}

void addNotification(const char *username, const char *message) {
    char path[MAX_PATH];
    getNotificationPath(path, sizeof(path), username);
    FILE *fp = fopen(path, "a");
    if (fp == NULL) return;
    char ts[20];
    getCurrentTimestamp(ts, sizeof(ts));
    fprintf(fp, "[%s] %s\n", ts, message);
    fclose(fp);
}

void viewNotifications(const char *currentUser) {
    char path[MAX_PATH];
    getNotificationPath(path, sizeof(path), currentUser);
    FILE *fp = fopen(path, "r");

    printf("\n");
    printCenteredLine('=', 36);
    printCentered("NOTIFICATIONS");
    printCenteredLine('=', 36);
    printf("\n");

    if (fp == NULL) {
        printCentered("No notifications.");
        return;
    }

    char line[MAX_LINE];
    int count = 0;
    while (fgets(line, sizeof(line), fp)) {
        printf("  %s", line);
        count++;
    }
    fclose(fp);

    if (count == 0) printCentered("No notifications.");
    printf("\n");
    printCenteredLine('-', 30);
    char temp[40];
    snprintf(temp, sizeof(temp), "%d notification(s)", count);
    printCentered(temp);

    /* Clear notifications after viewing */
    clearNotifications(currentUser);
    printf("\n[Info] Notifications have been marked as read.\n");
}

void clearNotifications(const char *currentUser) {
    char path[MAX_PATH];
    getNotificationPath(path, sizeof(path), currentUser);
    FILE *fp = fopen(path, "w");
    if (fp) { fclose(fp); }
}

void chatMenu(const char *loggedInUser) {
    int choice;
    char chatPath[MAX_PATH];
    char recipient[30];
    char input[MAX_LINE];

    snprintf(chatPath, MAX_PATH, "%schat.txt", appConfig.dataFolder);

    while (1) {
        system(CLEAR_SCREEN);
        printf("\n");
        printCenteredLine('=', 36);
        printCentered("CHAT & NOTIFICATIONS");
        printCenteredLine('=', 36);
        printf("                    1. Group Chat (Live)\n");
        printf("                    2. Send Direct Message\n");
        printf("                    3. Inbox (Received)\n");
        printf("                    4. Sent Messages\n");
        printf("                    5. View Notifications\n");
        printf("                    6. Back to Main Menu\n");
        printCenteredLine('=', 36);
        printf("                    Choice: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                system(CLEAR_SCREEN);
                viewGroupChat(chatPath, loggedInUser);
                break;
            case 2:
                system(CLEAR_SCREEN);
                printf("\n--- Send Direct Message ---\n");
                printf("(Press Enter on username to cancel)\n");
                printf("Send to (username): ");
                fgets(recipient, sizeof(recipient), stdin);
                recipient[strcspn(recipient, "\n")] = 0;
                if (strlen(recipient) == 0) {
                    printf("[Cancelled] Message not sent.\n");
                } else {
                    printf("Message (Enter to skip): ");
                    fgets(input, sizeof(input), stdin);
                    input[strcspn(input, "\n")] = 0;
                    sendDirectMessage(loggedInUser, recipient, input, chatPath);
                }
                printf("\n                    Press Enter to continue...");
                getchar();
                break;
            case 3:
                system(CLEAR_SCREEN);
                viewInbox(chatPath, loggedInUser);
                break;
            case 4:
                system(CLEAR_SCREEN);
                viewSentMessages(chatPath, loggedInUser);
                break;
            case 5:
                system(CLEAR_SCREEN);
                viewNotifications(loggedInUser);
                printf("\n                    Press Enter to continue...");
                getchar();
                break;
            case 6:
                return;
            default:
                printf("\n");
                printCentered("Invalid choice.");
        }
    }
}