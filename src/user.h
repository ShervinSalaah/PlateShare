/**
 * @file user.h
 * @brief User management interface
 * @author Member 2
 * 
 * Provides user registration, login, profile management, and CSV persistence.
 */

#ifndef USER_H
#define USER_H

#include "config.h"

/**
 * @brief Registers a new user with full input validation
 * @param users Array of users
 * @param count Pointer to current user count
 * @return 1 on success, 0 on failure
 */
int registerUser(User *users, int *count);

/**
 * @brief Authenticates a user by username and password
 * @param users Array of users
 * @param count Current user count
 * @param loggedInUser Buffer to receive the username on success
 * @return 1 on success, 0 on failure
 */
int loginUser(User *users, int count, char *loggedInUser);

/**
 * @brief Displays all registered users in a table
 * @param users Array of users
 * @param count Current user count
 */
void displayAllUsers(const User *users, int count);

/**
 * @brief Updates the profile of a specific user
 * @param users Array of users
 * @param count Current user count
 * @param username Username to update
 * @return 1 on success, 0 if not found
 */
int updateUserProfile(User *users, int count, const char *username);

/**
 * @brief Validates password strength (>=6 chars, 1 digit)
 * @param pwd Password to check
 * @return 1 if valid, 0 otherwise
 */
int validatePassword(const char *pwd);

/**
 * @brief Loads users from CSV file into memory
 * @param users Array to populate
 * @param count Pointer to store count
 */
void loadUsers(User *users, int *count);

/**
 * @brief Saves users array to CSV file
 * @param users Array to save
 * @param count Number of users
 */
void saveUsers(const User *users, int count);

/**
 * @brief User management sub-menu loop
 * @param loggedInUser Current username
 * @param loggedIn Pointer to login state flag
 */
void userMenu(char *loggedInUser, int *loggedIn);

#endif