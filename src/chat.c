#include "chat.h"
#include "core.h"
#include <ctype.h>

/* Helper: get notification file path */
static void getNotificationPath(char *path, size_t size, const char *username) {
    snprintf(path, size, "%snotifications_%s.txt", appConfig.dataFolder, username);
}

/* Helper: parse group chat message (no -> means group) */
static int isGroupMessage(const char *line) {
    /* Group messages don't contain " -> " */
    return strstr(line, " -> ") == NULL;
}

/* Helper: parse direct message to check if recipient matches */
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

/* Helper: parse direct message to check if sender matches */
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

/* ========== PUBLIC FUNCTIONS ========== */

void viewGroupChat(const char *chatFilePath, const char *currentUser) {
    char line[MAX_LINE];
    long lastSize = 0;
    FILE *fp;

    /* Get initial file size */
    fp = fopen(chatFilePath, "r");
    if (fp) { fseek(fp, 0, SEEK_END); lastSize = ftell(fp); fclose(fp); }

    printf("\n========================================\n");
    printf("  GROUP CHAT (type message + Enter, /q to quit, /dm user msg for direct)\n");
    printf("========================================\n\n");

    /* Print existing group messages */
    fp = fopen(chatFilePath, "r");
    if (fp) {
        while (fgets(line, sizeof(line), fp)) {
            if (isGroupMessage(line)) {
                printf("%s", line);
            }
        }
        fclose(fp);
    }

    /* Live loop: poll + allow input */
    while (1) {
        SLEEP(1);
        
        /* Check for new messages */
        fp = fopen(chatFilePath, "r");
        if (fp) {
            fseek(fp, 0, SEEK_END);
            long cur = ftell(fp);
            if (cur > lastSize) {
                fseek(fp, lastSize, SEEK_SET);
                while (fgets(line, sizeof(line), fp)) {
                    if (isGroupMessage(line)) {
                        printf("%s", line);
                    }
                }
                lastSize = cur;
            }
            fclose(fp);
        }

        /* Prompt user for input */
        printf("> ");
        char input[MAX_MSG];
        if (fgets(input, sizeof(input), stdin) == NULL) continue;
        input[strcspn(input, "\n")] = 0;

        if (strlen(input) == 0) continue;
        if (strcmp(input, "/q") == 0) {
            printf("Exiting group chat.\n");
            break;
        }

        /* Check for direct message command: /dm username message */
        if (strncmp(input, "/dm ", 4) == 0) {
            char *rest = input + 4;
            char *firstSpace = strchr(rest, ' ');
            if (firstSpace) {
                *firstSpace = '\0';
                char *recip = rest;
                char *msg = firstSpace + 1;
                sendDirectMessage(currentUser, recip, msg, chatFilePath);
            } else {
                printf("Usage: /dm username message\n");
            }
        } else {
            /* Group message */
            FILE *fpApp = fopen(chatFilePath, "a");
            if (fpApp) {
                char ts[20];
                getCurrentTimestamp(ts, sizeof(ts));
                fprintf(fpApp, "[%s] %s: %s\n", ts, currentUser, input);
                fclose(fpApp);
                printf("\033[1A\033[2K"); /* Clear the input line */
                printf("[%s] %s: %s\n", ts, currentUser, input);
            }
        }
    }
}

void sendDirectMessage(const char *sender, const char *recipient, const char *msg, const char *chatFilePath) {
    FILE *fp = fopen(chatFilePath, "a");
    if (fp == NULL) { printf("Error opening chat file.\n"); return; }
    char ts[20];
    getCurrentTimestamp(ts, sizeof(ts));
    fprintf(fp, "[%s] %s -> %s: %s\n", ts, sender, recipient, msg);
    fclose(fp);
    printf("Direct message sent to %s.\n", recipient);
    
    /* Also add notification for recipient */
    char notifMsg[MAX_MSG + 60];
    snprintf(notifMsg, sizeof(notifMsg), "New message from %s: %.100s", sender, msg);
    addNotification(recipient, notifMsg);
}

void viewInbox(const char *chatFilePath, const char *currentUser) {
    char line[MAX_LINE];
    long lastSize = 0;
    FILE *fp;

    fp = fopen(chatFilePath, "r");
    if (fp) { fseek(fp, 0, SEEK_END); lastSize = ftell(fp); fclose(fp); }

    printf("\n=== INBOX (Private Messages for %s) ===\n", currentUser);
    printf("(q to quit, new messages appear live)\n\n");

    fp = fopen(chatFilePath, "r");
    int hasMsgs = 0;
    if (fp) {
        while (fgets(line, sizeof(line), fp)) {
            if (isMessageForUser(line, currentUser) || isMessageFromUser(line, currentUser)) {
                if (!isGroupMessage(line)) {
                    printf("%s", line);
                    hasMsgs = 1;
                }
            }
        }
        fclose(fp);
    }
    if (!hasMsgs) printf("No private messages.\n");

    /* Live polling */
    while (1) {
        SLEEP(2);
        fp = fopen(chatFilePath, "r");
        if (fp) {
            fseek(fp, 0, SEEK_END);
            long cur = ftell(fp);
            if (cur > lastSize) {
                fseek(fp, lastSize, SEEK_SET);
                while (fgets(line, sizeof(line), fp)) {
                    if (isMessageForUser(line, currentUser) || isMessageFromUser(line, currentUser)) {
                        if (!isGroupMessage(line)) {
                            printf("%s", line);
                        }
                    }
                }
                lastSize = cur;
            }
            fclose(fp);
        }
        printf("(q to quit) ");
        char in[10]; fgets(in, 10, stdin);
        if (in[0] == 'q' || in[0] == 'Q') break;
    }
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
    if (fp == NULL) {
        printf("\n=== NOTIFICATIONS ===\n");
        printf("No notifications.\n");
        return;
    }
    printf("\n=== NOTIFICATIONS ===\n");
    char line[MAX_LINE];
    int count = 0;
    while (fgets(line, sizeof(line), fp)) {
        printf("%s", line);
        count++;
    }
    fclose(fp);
    if (count == 0) printf("No notifications.\n");
    printf("=== %d notification(s) ===\n", count);
}

void clearNotifications(const char *currentUser) {
    char path[MAX_PATH];
    getNotificationPath(path, sizeof(path), currentUser);
    FILE *fp = fopen(path, "w");
    if (fp) { fclose(fp); printf("Notifications cleared.\n"); }
}