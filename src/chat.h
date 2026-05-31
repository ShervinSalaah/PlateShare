#ifndef CHAT_H 
#define CHAT_H 
#include "config.h" 
 
void sendMessage(const char *sender, const char *msg, const char *chatFilePath); 
void viewChatHistory(const char *chatFilePath); 
void clearChat(const char *chatFilePath); 
void encryptMessage(char *msg); 
void decryptMessage(char *msg); 
 
#endif 
