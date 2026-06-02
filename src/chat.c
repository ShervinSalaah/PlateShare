/**
 * @file chat.c
 * @brief Chat and notification system implementation
 * @author Member 5 - Salaah
 * 
 * Features:
 * - Group chat with live polling and inline sending
 * - Direct messages stored per user pair (dm_alice_bob.txt)
 * - Chat list showing all conversation partners
 * - Individual chat history with any user
 * - Inbox (received DMs) and Sent folders
 * - System notifications for events
 */

#include "chat.h"
#include "core.h"
#include <ctype.h>

/* ========== Path Helpers ========== */

static void getNotificationPath(char *path, size_t size, const char *username) {
    snprintf(path, size, "%snotifications_%s.txt", appConfig.dataFolder, username);
}

/**
 * @brief Builds the file path for direct messages between two users
 * 
 * Creates a consistent filename by sorting usernames alphabetically.
 * Example: alice and bob -> dm_alice_bob.txt (always alice first)
 */
static void getDMPath(char *path, size_t size, const char *user1, const char *user2) {
    if (strcmp(user1, user2) < 0) {
        snprintf(path, size, "%sdm_%s_%s.txt", appConfig.dataFolder, user1, user2);
    } else {
        snprintf(path, size, "%sdm_%s_%s.txt", appConfig.dataFolder, user2, user1);
    }
}

/* ========== Message Type Checks ========== */

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

/* ========== Public Functions ========== */

int userExists(const char *username) {
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, username) == 0) return 1;
    }
    return 0;
}

/**
 * @brief Live group chat with inline sending
 * 
 * Shows all group messages and polls for new ones every second.
 * Commands:
 * - Type a message + Enter: sends to group
 * - /dm username message: sends direct message
 * - /q: quit group chat
 */
void viewGroupChat(const char *chatFilePath, const char *currentUser) {
    char line[MAX_LINE];
    long lastSize = 0;
    FILE *fp = fopen(chatFilePath, "r");
    if (fp) { fseek(fp, 0, SEEK_END); lastSize = ftell(fp); fclose(fp); }

    printf("\n");
    printCenteredLine('=', 40);
    printCentered("GROUP CHAT");
    printCentered("Type message + Enter | /dm name msg | /q quit");
    printCenteredLine('=', 40);
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
        if (strcmp(input, "/q") == 0) { printf("  Leaving group chat.\n"); break; }
        if (strncmp(input, "/dm ", 4) == 0) {
            char *rest = input + 4;
            char *firstSpace = strchr(rest, ' ');
            if (firstSpace) {
                *firstSpace = '\0';
                if (!userExists(rest)) {
                    printf("  User '%s' not found.\n", rest);
                } else {
                    sendDirectMessage(currentUser, rest, firstSpace + 1);
                }
            } else {
                printf("  Use: /dm username message\n");
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

/**
 * @brief Sends a direct message to another user
 * 
 * Stores the message in a dedicated file for this user pair.
 * Validates recipient exists and message is not empty.
 * Also sends a notification to the recipient.
 */
void sendDirectMessage(const char *sender, const char *recipient, const char *msg) {
    if (strlen(recipient) == 0) {
        printf("  Cancelled. No recipient.\n");
        return;
    }
    if (!userExists(recipient)) {
        printf("  User '%s' does not exist!\n", recipient);
        printf("  1. Try another name\n");
        printf("  2. Go back\n");
        printf("  Choice: ");
        char c; scanf(" %c", &c); getchar();
        return;
    }
    if (strlen(msg) == 0) {
        printf("  Cancelled. Empty message.\n");
        return;
    }

    char dmPath[MAX_PATH];
    getDMPath(dmPath, sizeof(dmPath), sender, recipient);
    FILE *fp = fopen(dmPath, "a");
    if (fp == NULL) { printf("  Error saving message.\n"); return; }
    char ts[20];
    getCurrentTimestamp(ts, sizeof(ts));
    fprintf(fp, "[%s] %s -> %s: %s\n", ts, sender, recipient, msg);
    fclose(fp);
    printf("  Message sent to %s.\n", recipient);

    char notifMsg[MAX_MSG + 60];
    snprintf(notifMsg, sizeof(notifMsg), "New message from %s: %.100s", sender, msg);
    addNotification(recipient, notifMsg);
}

/**
 * @brief Shows all users the current user has exchanged DMs with
 * 
 * Scans the data folder for dm_ files containing the user's name
 * and lists the other participants.
 */
void viewDirectChatList(const char *currentUser) {
    printf("\n--- Your Conversations ---\n");

    /* We check all possible users by scanning the users array */
    int found = 0;
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, currentUser) == 0) continue;

        char dmPath[MAX_PATH];
        getDMPath(dmPath, sizeof(dmPath), currentUser, users[i].username);
        FILE *fp = fopen(dmPath, "r");
        if (fp) {
            fclose(fp);
            printf("  Chat with: %s\n", users[i].username);
            found++;
        }
    }
    if (!found) printf("  No conversations yet.\n");
    printf("\n  Enter username to view chat (or press Enter to go back): ");
    char otherUser[30];
    fgets(otherUser, sizeof(otherUser), stdin);
    otherUser[strcspn(otherUser, "\n")] = 0;
    if (strlen(otherUser) > 0) {
        viewDirectChatHistory(currentUser, otherUser);
    }
}

/**
 * @brief Shows the complete chat history between two users
 * 
 * Reads the dedicated dm_ file for this user pair and
 * displays all messages in chronological order.
 */
void viewDirectChatHistory(const char *currentUser, const char *otherUser) {
    char dmPath[MAX_PATH];
    getDMPath(dmPath, sizeof(dmPath), currentUser, otherUser);

    printf("\n");
    printCenteredLine('=', 40);
    char title[80];
    snprintf(title, sizeof(title), "Chat: %s & %s", currentUser, otherUser);
    printCentered(title);
    printCenteredLine('=', 40);
    printf("\n");

    FILE *fp = fopen(dmPath, "r");
    if (fp == NULL) {
        printf("  No messages with %s yet.\n", otherUser);
    } else {
        char line[MAX_LINE];
        while (fgets(line, sizeof(line), fp)) {
            printf("%s", line);
        }
        fclose(fp);
    }

    /* Option to send a new message */
    printf("\n  Send a message? (type message or Enter to skip): ");
    char msg[MAX_MSG];
    fgets(msg, sizeof(msg), stdin);
    msg[strcspn(msg, "\n")] = 0;
    if (strlen(msg) > 0) {
        sendDirectMessage(currentUser, otherUser, msg);
    }

    printf("\n  Press Enter to continue..."); getchar();
}

/**
 * @brief Shows all received direct messages from all conversations
 * 
 * Scans all dm_ files involving the current user and displays
 * only messages addressed TO the current user.
 */
void viewInbox(const char *currentUser) {
    printf("\n");
    printCenteredLine('=', 40);
    printCentered("INBOX (Messages You Received)");
    printCenteredLine('=', 40);
    printf("\n");

    int hasMsgs = 0;
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, currentUser) == 0) continue;

        char dmPath[MAX_PATH];
        getDMPath(dmPath, sizeof(dmPath), currentUser, users[i].username);
        FILE *fp = fopen(dmPath, "r");
        if (fp) {
            char line[MAX_LINE];
            while (fgets(line, sizeof(line), fp)) {
                if (isMessageForUser(line, currentUser)) {
                    printf("%s", line);
                    hasMsgs = 1;
                }
            }
            fclose(fp);
        }
    }
    if (!hasMsgs) printf("  No received messages.\n");
    pauseScreen("");
}

/**
 * @brief Shows all sent direct messages across all conversations
 */
void viewSentMessages(const char *currentUser) {
    printf("\n");
    printCenteredLine('=', 40);
    printCentered("SENT (Messages You Sent)");
    printCenteredLine('=', 40);
    printf("\n");

    int hasMsgs = 0;
    for (int i = 0; i < userCount; i++) {
        if (strcmp(users[i].username, currentUser) == 0) continue;

        char dmPath[MAX_PATH];
        getDMPath(dmPath, sizeof(dmPath), currentUser, users[i].username);
        FILE *fp = fopen(dmPath, "r");
        if (fp) {
            char line[MAX_LINE];
            while (fgets(line, sizeof(line), fp)) {
                if (isMessageFromUser(line, currentUser)) {
                    printf("%s", line);
                    hasMsgs = 1;
                }
            }
            fclose(fp);
        }
    }
    if (!hasMsgs) printf("  No sent messages.\n");
    pauseScreen("");
}

/**
 * @brief Adds a notification for a user
 * 
 * Appends a timestamped message to the user's notification file.
 */
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

/**
 * @brief Shows all notifications and marks them as read
 * 
 * After displaying, clears the notification file so the
 * count resets to zero on the main menu.
 */
void viewNotifications(const char *currentUser) {
    char path[MAX_PATH];
    getNotificationPath(path, sizeof(path), currentUser);
    FILE *fp = fopen(path, "r");

    printf("\n");
    printCenteredLine('=', 40);
    printCentered("YOUR ALERTS");
    printCenteredLine('=', 40);
    printf("\n");

    if (fp == NULL) {
        printCentered("No alerts.");
        return;
    }

    char line[MAX_LINE];
    int count = 0;
    while (fgets(line, sizeof(line), fp)) {
        printf("  %s", line);
        count++;
    }
    fclose(fp);

    if (count == 0) printCentered("No alerts.");
    printf("\n");
    printCenteredLine('-', 30);
    char temp[40];
    snprintf(temp, sizeof(temp), "%d alert(s)", count);
    printCentered(temp);

    /* Mark as read (clear the file) */
    clearNotifications(currentUser);
    printf("\n  [Alerts marked as read]\n");
}

/**
 * @brief Clears all notifications for a user
 */
void clearNotifications(const char *currentUser) {
    char path[MAX_PATH];
    getNotificationPath(path, sizeof(path), currentUser);
    FILE *fp = fopen(path, "w");
    if (fp) fclose(fp);
}

/**
 * @brief Messages and alerts sub-menu
 */
void chatMenu(const char *loggedInUser) {
    int choice;
    char chatPath[MAX_PATH];
    char recipient[30];
    char input[MAX_LINE];

    snprintf(chatPath, MAX_PATH, "%schat.txt", appConfig.dataFolder);

    while (1) {
        system(CLEAR_SCREEN);
        printf("\n");
        printCenteredLine('=', 40);
        printCentered("MESSAGES & ALERTS");
        printCenteredLine('=', 40);
        printf("                    1. Group Chat (Everyone)\n");
        printf("                    2. Send a Message\n");
        printf("                    3. My Conversations\n");
        printf("                    4. Inbox (Received)\n");
        printf("                    5. Sent Messages\n");
        printf("                    6. View Alerts\n");
        printf("                    7. Go Back\n");
        printCenteredLine('=', 40);
        printf("                    Your choice: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                system(CLEAR_SCREEN);
                viewGroupChat(chatPath, loggedInUser);
                break;
            case 2:
                system(CLEAR_SCREEN);
                printf("\n--- Send a Message ---\n");
                printf("  (Press Enter on username to cancel)\n");
                printf("  Send to: ");
                fgets(recipient, sizeof(recipient), stdin);
                recipient[strcspn(recipient, "\n")] = 0;
                if (strlen(recipient) == 0) {
                    printf("  Cancelled.\n"); pauseScreen("");
                } else {
                    printf("  Message (Enter to skip): ");
                    fgets(input, sizeof(input), stdin);
                    input[strcspn(input, "\n")] = 0;
                    sendDirectMessage(loggedInUser, recipient, input);
                    pauseScreen("");
                }
                break;
            case 3:
                system(CLEAR_SCREEN);
                viewDirectChatList(loggedInUser);
                break;
            case 4:
                system(CLEAR_SCREEN);
                viewInbox(loggedInUser);
                break;
            case 5:
                system(CLEAR_SCREEN);
                viewSentMessages(loggedInUser);
                break;
            case 6:
                system(CLEAR_SCREEN);
                viewNotifications(loggedInUser);
                pauseScreen("");
                break;
            case 7:
                return;
            default:
                printf("\n"); printCentered("Wrong choice!"); pauseScreen("");
        }
    }
}