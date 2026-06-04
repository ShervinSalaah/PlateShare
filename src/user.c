/**
 * @file user.c
 * @brief User account management implementation
 * @author 254187H SAHITHIYAN J
 * 
 * Handles all user-related operations for PlateShare Pro:
 * - Creating new accounts with input validation
 * - Signing in with username and password authentication
 * - Viewing the list of all community members
 * - Updating profile information (name and email)
 * - Loading and saving user data to users.csv for persistence
 * 
 * Data Storage:
 * - Users are stored in users.csv with format: username,password,fullname,email
 * - The global users[] array holds all users in memory during runtime
 * - userCount tracks how many users are currently registered
 */

#include "user.h"
#include "core.h"
#include <ctype.h>

/* ========== File I/O Functions ========== */

/**
 * @brief Loads all users from users.csv into the in-memory array
 * 
 * Opens the users.csv file from the configured data folder.
 * Reads each line, parses the comma-separated fields, and
 * populates the users array. Handles both the current format
 * (4 fields: username,password,fullname,email) and the old
 * format (5 fields with role) for backward compatibility.
 * 
 * If the file doesn't exist, sets count to 0 (empty database).
 * The program will create the file on first registration.
 * 
 * @param users Pointer to the global users array to populate
 * @param count Pointer to store the number of loaded users
 */
void loadUsers(User *users, int *count) {
    /* Build the full file path using the configured data folder */
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "%susers.csv", appConfig.dataFolder);
    
    /* Try to open the file for reading */
    FILE *fp = fopen(path, "r");
    if (fp == NULL) {
        /* File doesn't exist yet - start with empty database */
        *count = 0;
        return;
    }
    
    *count = 0;
    char line[MAX_LINE];
    
    /* Read the file line by line until end of file or array is full */
    while (fgets(line, sizeof(line), fp) && *count < MAX_USERS) {
        /* Remove the trailing newline character */
        line[strcspn(line, "\n")] = 0;
        
        /* Try parsing with current format: username,password,fullname,email (4 fields) */
        if (sscanf(line, "%[^,],%[^,],%[^,],%s",
            users[*count].username, users[*count].password,
            users[*count].fullname, users[*count].email) == 4) {
            (*count)++;
        }
        /* Fallback: try old format with role field for backward compatibility */
        else {
            char dummyRole[10];
            if (sscanf(line, "%[^,],%[^,],%[^,],%[^,],%s",
                users[*count].username, users[*count].password,
                users[*count].fullname, dummyRole,
                users[*count].email) >= 4) {
                (*count)++;
            }
        }
    }
    fclose(fp);
}

/**
 * @brief Saves all users from the in-memory array to users.csv
 * 
 * Opens the users.csv file for writing (creates it if it doesn't exist,
 * overwrites if it does). Writes each user as a comma-separated line
 * in the format: username,password,fullname,email
 * 
 * This function is called after every change to user data:
 * - New user registration
 * - Profile updates
 * 
 * @param users Pointer to the global users array
 * @param count Current number of registered users
 */
void saveUsers(const User *users, int count) {
    /* Build the full file path */
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "%susers.csv", appConfig.dataFolder);
    
    /* Open for writing - "w" mode creates the file if it doesn't exist */
    FILE *fp = fopen(path, "w");
    if (fp == NULL) return;
    
    /* Write each user as a CSV line */
    for (int i = 0; i < count; i++) {
        fprintf(fp, "%s,%s,%s,%s\n",
            users[i].username, users[i].password,
            users[i].fullname, users[i].email);
    }
    fclose(fp);
}

/* ========== Validation Functions ========== */

/**
 * @brief Checks if a password meets the minimum security requirements
 * 
 * Password rules:
 * - Must be at least 6 characters long
 * - Must contain at least 1 digit (0-9)
 * 
 * These are simple rules appropriate for a community sharing app.
 * In a production system, stronger requirements would be used.
 * 
 * @param pwd The password string to validate
 * @return 1 if the password is valid, 0 if it fails any check
 */
int validatePassword(const char *pwd) {
    /* Check minimum length */
    if (strlen(pwd) < 6) return 0;
    
    /* Check for at least one digit */
    for (int i = 0; pwd[i]; i++) {
        if (isdigit(pwd[i])) return 1;  /* Found a digit - valid */
    }
    
    return 0;  /* No digit found - invalid */
}

/* ========== User Operations ========== */

/**
 * @brief Creates a new user account
 * 
 * Guides the user through entering their account details:
 * 1. Username - must be unique (not already taken), cannot be empty
 * 2. Password - must be 6+ characters with at least 1 digit
 * 3. Full name - cannot be empty
 * 4. Email - cannot be empty
 * 
 * Each field is validated immediately. If invalid, the user is
 * asked to re-enter until they provide valid input. After all
 * fields are accepted, the new user is saved to the array
 * and written to users.csv.
 * 
 * In PlateShare Pro, any member can both share food and request
 * food. There is no role separation - everyone is equal in the
 * community.
 * 
 * @param users Pointer to the global users array
 * @param count Pointer to the current user count (incremented on success)
 * @return 1 if registration was successful, 0 if failed or cancelled
 */
int registerUser(User *users, int *count) {
    /* Check if the user database is full */
    if (*count >= MAX_USERS) {
        printf("\n  Sorry! The system has reached its maximum capacity.\n");
        printf("  Cannot create more accounts at this time.\n");
        pauseScreen("");
        return 0;
    }
    
    User u;  /* Temporary user structure to build before saving */
    
    printf("\n--- Create New Account ---\n");
    printf("  Join the PlateShare community!\n\n");
    printf("  (Type '0' and press Enter at any prompt to cancel)\n\n");

    /* ===== USERNAME ===== */
    /* Required field - must be unique and not empty */
    while (1) {
        printf("  Choose a username: ");
        fgets(u.username, 30, stdin);
        u.username[strcspn(u.username, "\n")] = 0;
        if (strcmp(u.username, "0") == 0) {
            printf("  Registration cancelled.\n"); pauseScreen(""); return 0;
        }
        if (strlen(u.username) > 0) break;  /* Valid - not empty */
        printf("  Please enter a username. It cannot be empty.\n");
    }
    
    /* Check for duplicate username */
    for (int i = 0; i < *count; i++) {
        if (strcmp(users[i].username, u.username) == 0) {
            printf("  This username is already taken by another member.\n");
            printf("  Please choose a different username.\n");
            pauseScreen("");
            return 0;
        }
    }

    /* ===== PASSWORD ===== */
    /* Required field - must be strong (6+ chars, 1 digit) */
    while (1) {
        printf("  Password (6+ characters, at least 1 number): ");
        fgets(u.password, 30, stdin);
        u.password[strcspn(u.password, "\n")] = 0;
        if (strcmp(u.password, "0") == 0) {
            printf("  Registration cancelled.\n"); pauseScreen(""); return 0;
        }
        if (strlen(u.password) == 0) {
            printf("  Password cannot be empty. Please enter a password.\n");
            continue;
        }
        if (!validatePassword(u.password)) {
            printf("  Password must be at least 6 characters long and contain\n");
            printf("  at least one number (0-9). Please try again.\n");
            continue;
        }
        break;  /* Password is valid */
    }

    /* ===== FULL NAME ===== */
    /* Required field - cannot be empty */
    while (1) {
        printf("  Your full name: ");
        fgets(u.fullname, 50, stdin);
        u.fullname[strcspn(u.fullname, "\n")] = 0;
        if (strcmp(u.fullname, "0") == 0) {
            printf("  Registration cancelled.\n"); pauseScreen(""); return 0;
        }
        if (strlen(u.fullname) > 0) break;
        printf("  Name cannot be empty. Please enter your name.\n");
    }

    /* ===== EMAIL ===== */
    /* Required field - cannot be empty */
    while (1) {
        printf("  Email address: ");
        fgets(u.email, 40, stdin);
        u.email[strcspn(u.email, "\n")] = 0;
        if (strcmp(u.email, "0") == 0) {
            printf("  Registration cancelled.\n"); pauseScreen(""); return 0;
        }
        if (strlen(u.email) > 0) break;
        printf("  Email cannot be empty. Please enter your email.\n");
    }

    /* ===== SAVE ===== */
    /* All fields validated - add to array and save to file */
    users[*count] = u;
    (*count)++;
    saveUsers(users, *count);
    
    printf("\n");
    printCenteredLine('=', 40);
    char welcome[60];
    snprintf(welcome, sizeof(welcome), "Welcome to PlateShare, %s!", u.fullname);
    printCentered(welcome);
    printCentered("Your account has been created.");
    printCenteredLine('=', 40);
    pauseScreen("");
    return 1;
}

/**
 * @brief Authenticates a user by checking username and password
 * 
 * Prompts for username and password, then searches the users array
 * for a matching pair. If found, stores the username in the
 * loggedInUser buffer so other modules know who is signed in.
 * 
 * Security note: Passwords are compared in plain text. This is
 * acceptable for a demonstration but would need hashing in a
 * production system.
 * 
 * @param users Pointer to the global users array
 * @param count Current number of registered users
 * @param loggedInUser Buffer to store the username on successful login
 * @return 1 if login was successful, 0 if credentials were wrong
 */
int loginUser(User *users, int count, char *loggedInUser) {
    char u[30], p[30];
    
    printf("\n--- Sign In ---\n");
    printf("  (Type '0' to cancel)\n\n");
    
    /* Get username */
    printf("  Username: ");
    fgets(u, 30, stdin);
    u[strcspn(u, "\n")] = 0;
    if (strcmp(u, "0") == 0) { printf("  Sign in cancelled.\n"); pauseScreen(""); return 0; }
    
    /* Get password */
    printf("  Password: ");
    fgets(p, 30, stdin);
    p[strcspn(p, "\n")] = 0;
    if (strcmp(p, "0") == 0) { printf("  Sign in cancelled.\n"); pauseScreen(""); return 0; }

    /* Search for matching credentials */
    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].username, u) == 0 &&
            strcmp(users[i].password, p) == 0) {
            /* Match found - set the logged-in username */
            strcpy(loggedInUser, u);
            printf("\n");
            printCenteredLine('=', 40);
            char welcome[60];
            snprintf(welcome, sizeof(welcome), "Welcome back, %s!", users[i].fullname);
            printCentered(welcome);
            printCenteredLine('=', 40);
            pauseScreen("");
            return 1;
        }
    }
    
    /* No match found */
    printf("\n  Wrong username or password. Please try again.\n");
    pauseScreen("");
    return 0;
}

/**
 * @brief Displays all registered community members in a formatted table
 * 
 * Shows a numbered list with username, full name, and email address
 * for every registered user. If no users exist yet, displays a
 * friendly message.
 * 
 * @param users Pointer to the global users array
 * @param count Current number of registered users
 */
void displayAllUsers(const User *users, int count) {
    if (count == 0) {
        printf("\n  No community members yet. Be the first to join!\n");
        return;
    }
    
    printf("\n");
    printCenteredLine('=', 40);
    printCentered("COMMUNITY MEMBERS");
    printCenteredLine('=', 40);
    printf("\n");
    
    /* Table header */
    printf("  %-5s %-20s %-35s %-25s\n", "#", "Username", "Full Name", "Email");
    printf("  ------------------------------------------------------------------------------------------\n");
    
    /* Print each user */
    for (int i = 0; i < count; i++) {
        printf("  %-5d %-20s %-35s %-25s\n",
            i + 1, users[i].username, users[i].fullname, users[i].email);
    }
}

/**
 * @brief Updates the logged-in user's profile information
 * 
 * Allows the user to change their full name and email address.
 * Shows the current values first, then asks for new values.
 * The user can press Enter to keep the current value unchanged.
 * 
 * After updating, the changes are immediately saved to users.csv.
 * 
 * @param users Pointer to the global users array
 * @param count Current number of registered users
 * @param username The username of the currently logged-in user
 * @return 1 if the profile was updated, 0 if user was not found
 */
int updateUserProfile(User *users, int count, const char *username) {
    /* Find the user in the array */
    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].username, username) == 0) {
            char in[50];
            
            printf("\n--- Update Your Profile ---\n");
            
            /* Update full name */
            printf("  Current name: %s\n", users[i].fullname);
            printf("  New name (press Enter to keep the same): ");
            fgets(in, 50, stdin);
            if (in[0] != '\n') {
                /* User typed something - update the name */
                in[strcspn(in, "\n")] = 0;
                strcpy(users[i].fullname, in);
            }
            
            /* Update email */
            printf("  Current email: %s\n", users[i].email);
            printf("  New email (press Enter to keep the same): ");
            fgets(in, 40, stdin);
            if (in[0] != '\n') {
                /* User typed something - update the email */
                in[strcspn(in, "\n")] = 0;
                strcpy(users[i].email, in);
            }
            
            /* Save changes to file */
            saveUsers(users, count);
            printf("\n  Your profile has been updated!\n");
            pauseScreen("");
            return 1;
        }
    }
    printf("\n  User not found. Something went wrong.\n");
    pauseScreen("");
    return 0;
}

/* ========== User Menu ========== */

/**
 * @brief Displays the account management sub-menu
 * 
 * Provides options to:
 * 1. See all community members
 * 2. Update own profile
 * 3. Go back to main menu
 * 
 * Runs in a loop until the user chooses to go back.
 * 
 * @param loggedInUser The username of the currently signed-in user
 * @param loggedIn Pointer to the login state flag (1 = logged in)
 */
void userMenu(char *loggedInUser, int *loggedIn) {
    int choice;
    
    while (1) {
        system(CLEAR_SCREEN);
        printf("\n");
        printCenteredLine('=', 40);
        printCentered("MY ACCOUNT");
        printCenteredLine('=', 40);
        printf("                    1. See all members\n");
        printf("                    2. Update my profile\n");
        printf("                    3. Go back\n");
        printCenteredLine('=', 40);
        printf("                    Your choice: ");
        scanf("%d", &choice);
        getchar();

        switch (choice) {
            case 1:
                system(CLEAR_SCREEN);
                loadUsers(users, &userCount);
                displayAllUsers(users, userCount);
                pauseScreen("");
                break;
            case 2:
                system(CLEAR_SCREEN);
                updateUserProfile(users, userCount, loggedInUser);
                break;
            case 3:
                return;
            default:
                printf("\n");
                printCentered("That's not a valid option. Please try again.");
                pauseScreen("");
        }
    }
}