#include "chat.h"
#include "core.h"

void sendMessage(const char *sender, const char *msg, const char *chatFilePath) {
    FILE *fp = fopen(chatFilePath, "a");
    if (fp == NULL) { printf("Error opening chat file.\n"); return; }
    char ts[20];
    getCurrentTimestamp(ts, sizeof(ts));
    fprintf(fp, "[%s] %s: %s\n", ts, sender, msg);
    fclose(fp);
    printf("Sent.\n");
}

void viewChatHistory(const char *chatFilePath) {
    char line[MAX_LINE];
    long lastSize = 0;
    FILE *fp = fopen(chatFilePath, "r");
    if (fp) { fseek(fp, 0, SEEK_END); lastSize = ftell(fp); fclose(fp); }
    printf("\n=== Chat History ===\n");
    fp = fopen(chatFilePath, "r");
    if (fp) { while (fgets(line, sizeof(line), fp)) printf("%s", line); fclose(fp); }
    else printf("No messages yet.\n");
    printf("=== Live (q to quit) ===\n");
    while (1) {
        SLEEP(2);
        fp = fopen(chatFilePath, "r");
        if (fp) {
            fseek(fp, 0, SEEK_END);
            long cur = ftell(fp);
            if (cur > lastSize) { fseek(fp, lastSize, SEEK_SET); while (fgets(line, sizeof(line), fp)) printf("%s", line); lastSize = cur; }
            fclose(fp);
        }
        printf("(q to quit) ");
        char in[10]; fgets(in, 10, stdin);
        if (in[0] == 'q' || in[0] == 'Q') break;
    }
}

void clearChat(const char *chatFilePath) {
    FILE *fp = fopen(chatFilePath, "w");
    if (fp) { fclose(fp); printf("Cleared.\n"); }
}

void encryptMessage(char *msg) {
    for (int i = 0; msg[i]; i++) {
        char c = msg[i];
        if (c >= 'a' && c <= 'z') msg[i] = (c - 'a' + 3) % 26 + 'a';
        else if (c >= 'A' && c <= 'Z') msg[i] = (c - 'A' + 3) % 26 + 'A';
    }
}

void decryptMessage(char *msg) {
    for (int i = 0; msg[i]; i++) {
        char c = msg[i];
        if (c >= 'a' && c <= 'z') msg[i] = (c - 'a' + 23) % 26 + 'a';
        else if (c >= 'A' && c <= 'Z') msg[i] = (c - 'A' + 23) % 26 + 'A';
    }
}