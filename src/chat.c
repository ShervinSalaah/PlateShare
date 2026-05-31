#include "chat.h"
#include "core.h"
#include <ctype.h>

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
    if (fp) { while (fgets(line, sizeof(line), fp)) if (isGroupMessage(line)) printf("%s", line); fclose(fp); }
    while (1) {
        SLEEP(1);
        fp = fopen(chatFilePath, "r");
        if (fp) {
            fseek(fp, 0, SEEK_END); long cur = ftell(fp);
            if (cur > lastSize) { fseek(fp, lastSize, SEEK_SET); while (fgets(line, sizeof(line), fp)) if (isGroupMessage(line)) printf("%s", line); lastSize = cur; }
            fclose(fp);
        }
        printf("> ");
        char input[MAX_MSG];
        if (fgets(input, sizeof(input), stdin) == NULL) continue;
        input[strcspn(input, "\n")] = 0;
        if (strlen(input) == 0) continue;
        if (strcmp(input, "/q") == 0) { printf("Exiting group chat.\n"); break; }
        if (strncmp(input, "/dm ", 4) == 0) {
            char *rest = input + 4;
            char *firstSpace = strchr(rest, ' ');
            if (firstSpace) { *firstSpace = '\0'; sendDirectMessage(currentUser, rest, firstSpace + 1, chatFilePath); }
            else printf("Usage: /dm username message\n");
        } else {
            FILE *fpApp = fopen(chatFilePath, "a");
            if (fpApp) { char ts[20]; getCurrentTimestamp(ts, sizeof(ts)); fprintf(fpApp, "[%s] %s: %s\n", ts, currentUser, input); fclose(fpApp); }
        }
    }
}

void sendDirectMessage(const char *sender, const char *recipient, const char *msg, const char *chatFilePath) {
    FILE *fp = fopen(chatFilePath, "a");
    if (fp == NULL) { printf("Error opening chat file.\n"); return; }
    char ts[20]; getCurrentTimestamp(ts, sizeof(ts));
    fprintf(fp, "[%s] %s -> %s: %s\n", ts, sender, recipient, msg);
    fclose(fp);
    printf("Direct message sent to %s.\n", recipient);
    char notifMsg[MAX_MSG + 60];
    snprintf(notifMsg, sizeof(notifMsg), "New message from %s: %.100s", sender, msg);
    addNotification(recipient, notifMsg);
}

void viewInbox(const char *chatFilePath, const char *currentUser) {
    char line[MAX_LINE]; long lastSize = 0;
    FILE *fp = fopen(chatFilePath, "r");
    if (fp) { fseek(fp, 0, SEEK_END); lastSize = ftell(fp); fclose(fp); }
    printf("\n");
    printCenteredLine('=', 36);
    printCentered("INBOX (Private Messages)");
    printCentered("q to quit");
    printCenteredLine('=', 36);
    printf("\n");
    fp = fopen(chatFilePath, "r");
    int hasMsgs = 0;
    if (fp) { while (fgets(line, sizeof(line), fp)) if ((isMessageForUser(line, currentUser) || isMessageFromUser(line, currentUser)) && !isGroupMessage(line)) { printf("%s", line); hasMsgs = 1; } fclose(fp); }
    if (!hasMsgs) printf("No private messages.\n");
    while (1) {
        SLEEP(2);
        fp = fopen(chatFilePath, "r");
        if (fp) { fseek(fp, 0, SEEK_END); long cur = ftell(fp); if (cur > lastSize) { fseek(fp, lastSize, SEEK_SET); while (fgets(line, sizeof(line), fp)) if ((isMessageForUser(line, currentUser) || isMessageFromUser(line, currentUser)) && !isGroupMessage(line)) printf("%s", line); lastSize = cur; } fclose(fp); }
        printf("(q to quit) "); char in[10]; fgets(in, 10, stdin);
        if (in[0] == 'q' || in[0] == 'Q') break;
    }
}

void addNotification(const char *username, const char *message) {
    char path[MAX_PATH]; getNotificationPath(path, sizeof(path), username);
    FILE *fp = fopen(path, "a");
    if (fp == NULL) return;
    char ts[20]; getCurrentTimestamp(ts, sizeof(ts));
    fprintf(fp, "[%s] %s\n", ts, message);
    fclose(fp);
}

void viewNotifications(const char *currentUser) {
    char path[MAX_PATH]; getNotificationPath(path, sizeof(path), currentUser);
    FILE *fp = fopen(path, "r");
    printf("\n");
    printCenteredLine('=', 36);
    printCentered("NOTIFICATIONS");
    printCenteredLine('=', 36);
    printf("\n");
    if (fp == NULL) { printCentered("No notifications."); return; }
    char line[MAX_LINE]; int count = 0;
    while (fgets(line, sizeof(line), fp)) { printf("  %s", line); count++; }
    fclose(fp);
    if (count == 0) printCentered("No notifications.");
    printf("\n"); printCenteredLine('-', 30);
    char temp[40]; snprintf(temp, sizeof(temp), "%d notification(s)", count); printCentered(temp);
}

void clearNotifications(const char *currentUser) {
    char path[MAX_PATH]; getNotificationPath(path, sizeof(path), currentUser);
    FILE *fp = fopen(path, "w");
    if (fp) { fclose(fp); printCentered("Notifications cleared."); }
}

void chatMenu(const char *loggedInUser) {
    int choice; char chatPath[MAX_PATH]; char recipient[30]; char input[MAX_LINE];
    snprintf(chatPath, MAX_PATH, "%schat.txt", appConfig.dataFolder);
    while (1) {
        system(CLEAR_SCREEN);
        printf("\n");
        printCenteredLine('=', 36);
        printCentered("CHAT & NOTIFICATIONS");
        printCenteredLine('=', 36);
        printf("                    1. Group Chat (Live)\n");
        printf("                    2. Send Direct Message\n");
        printf("                    3. Inbox (Private Messages)\n");
        printf("                    4. View Notifications\n");
        printf("                    5. Clear Notifications\n");
        printf("                    6. Back to Main Menu\n");
        printCenteredLine('=', 36);
        printf("                    Choice: ");
        scanf("%d", &choice); getchar();
        switch (choice) {
            case 1: system(CLEAR_SCREEN); viewGroupChat(chatPath, loggedInUser); break;
            case 2:
                system(CLEAR_SCREEN); printf("\n--- Send Direct Message ---\nSend to (username): ");
                fgets(recipient, sizeof(recipient), stdin); recipient[strcspn(recipient, "\n")] = 0;
                printf("Message: "); fgets(input, sizeof(input), stdin); input[strcspn(input, "\n")] = 0;
                sendDirectMessage(loggedInUser, recipient, input, chatPath);
                printf("\n                    Press Enter to continue..."); getchar(); break;
            case 3: system(CLEAR_SCREEN); viewInbox(chatPath, loggedInUser); break;
            case 4: system(CLEAR_SCREEN); viewNotifications(loggedInUser); printf("\n                    Press Enter to continue..."); getchar(); break;
            case 5: clearNotifications(loggedInUser); printf("\n                    Press Enter to continue..."); getchar(); break;
            case 6: return;
            default: printf("\n"); printCentered("Invalid choice.");
        }
    }
}