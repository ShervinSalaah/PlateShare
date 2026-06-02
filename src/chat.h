/**
 * @file chat.h
 * @brief Chat and notification system interface
 * @author Member 5 - Salaah
 * 
 * Provides group chat, direct messaging with per-user history,
 * inbox/sent folders, and notification management.
 * 
 * Direct messages are stored in separate files per user pair
 * to enable individual chat history viewing.
 */

#ifndef CHAT_H
#define CHAT_H

#include "config.h"

/* Group chat */
void viewGroupChat(const char *chatFilePath, const char *currentUser);

/* Direct messaging with per-user history */
void sendDirectMessage(const char *sender, const char *recipient, const char *msg);
void viewDirectChatList(const char *currentUser);
void viewDirectChatHistory(const char *currentUser, const char *otherUser);

/* Inbox and Sent folders */
void viewInbox(const char *currentUser);
void viewSentMessages(const char *currentUser);

/* Notifications */
void addNotification(const char *username, const char *message);
void viewNotifications(const char *currentUser);
void clearNotifications(const char *currentUser);

/* Menu */
void chatMenu(const char *loggedInUser);

/* Utility */
int userExists(const char *username);

#endif