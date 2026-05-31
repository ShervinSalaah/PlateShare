#ifndef CHAT_H
#define CHAT_H

#include "config.h"

/* Group chat with live view and inline send */
void viewGroupChat(const char *chatFilePath, const char *currentUser);

/* Direct message to a specific user */
void sendDirectMessage(const char *sender, const char *recipient, const char *msg, const char *chatFilePath);

/* View private messages (inbox) for current user */
void viewInbox(const char *chatFilePath, const char *currentUser);

/* Add a notification for a user (called by other modules) */
void addNotification(const char *username, const char *message);

/* View notification panel for current user */
void viewNotifications(const char *currentUser);

/* Clear notifications for current user */
void clearNotifications(const char *currentUser);

#endif