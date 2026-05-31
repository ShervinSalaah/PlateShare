#ifndef USER_H
#define USER_H
#include "config.h"

int registerUser(User *users, int *count);
int loginUser(User *users, int count, char *loggedInUser);
void displayAllUsers(const User *users, int count);
int updateUserProfile(User *users, int count, const char *username);
int validatePassword(const char *pwd);
void loadUsers(User *users, int *count);
void saveUsers(const User *users, int count);

#endif