#ifndef CHAT_H
#define CHAT_H

#include "config.h"

void viewGroupChat(const char *chatFilePath, const char *currentUser);
void sendDirectMessage(const char *sender, const char *recipient, const char *msg, const char *chatFilePath);
void viewInbox(const char *chatFilePath, const char *currentUser);
void addNotification(const char *username, const char *message);
void viewNotifications(const char *currentUser);
void clearNotifications(const char *currentUser);
void chatMenu(const char *loggedInUser);

#endif