/**
 * @file user.c
 * @brief User management implementation
 * @author Member 2
 */

#include "user.h"
#include "core.h"
#include <ctype.h>

void loadUsers(User *users, int *count) {
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "%susers.csv", appConfig.dataFolder);
    FILE *fp = fopen(path, "r");
    if (fp == NULL) { *count = 0; return; }
    *count = 0;
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), fp) && *count < MAX_USERS) {
        line[strcspn(line, "\n")] = 0;
        if (sscanf(line, "%[^,],%[^,],%[^,],%[^,],%s",
            users[*count].username, users[*count].password,
            users[*count].fullname, users[*count].role,
            users[*count].email) == 5) (*count)++;
    }
    fclose(fp);
}

void saveUsers(const User *users, int count) {
    char path[MAX_PATH];
    snprintf(path, MAX_PATH, "%susers.csv", appConfig.dataFolder);
    FILE *fp = fopen(path, "w");
    if (fp == NULL) return;
    for (int i = 0; i < count; i++)
        fprintf(fp, "%s,%s,%s,%s,%s\n", users[i].username, users[i].password,
            users[i].fullname, users[i].role, users[i].email);
    fclose(fp);
}

int validatePassword(const char *pwd) {
    if (strlen(pwd) < 6) return 0;
    for (int i = 0; pwd[i]; i++)
        if (isdigit(pwd[i])) return 1;
    return 0;
}

static int validateRole(const char *role) {
    if (strcmp(role, "donor") == 0) return 1;
    if (strcmp(role, "receiver") == 0) return 1;
    return 0;
}

int registerUser(User *users, int *count) {
    if (*count >= MAX_USERS) {
        printf("\n[Error] User database is full! Cannot register more users.\n");
        return 0;
    }
    User u;

    while (1) {
        printf("Username: ");
        fgets(u.username, 30, stdin); u.username[strcspn(u.username, "\n")] = 0;
        if (strlen(u.username) > 0) break;
        printf("[Warning] Username cannot be empty!\n");
    }
    for (int i = 0; i < *count; i++)
        if (strcmp(users[i].username, u.username) == 0) {
            printf("[Error] Username '%s' already exists!\n", u.username);
            return 0;
        }
    while (1) {
        printf("Password (6+ chars, at least 1 digit): ");
        fgets(u.password, 30, stdin); u.password[strcspn(u.password, "\n")] = 0;
        if (strlen(u.password) == 0) { printf("[Warning] Password cannot be empty!\n"); continue; }
        if (!validatePassword(u.password)) { printf("[Error] Must be 6+ characters with at least 1 digit!\n"); continue; }
        break;
    }
    while (1) {
        printf("Full name: ");
        fgets(u.fullname, 50, stdin); u.fullname[strcspn(u.fullname, "\n")] = 0;
        if (strlen(u.fullname) > 0) break;
        printf("[Warning] Full name cannot be empty!\n");
    }
    while (1) {
        printf("Role (donor/receiver): ");
        fgets(u.role, 10, stdin); u.role[strcspn(u.role, "\n")] = 0;
        if (strlen(u.role) == 0) { printf("[Warning] Role cannot be empty!\n"); continue; }
        if (!validateRole(u.role)) { printf("[Error] Invalid role! Choose 'donor' or 'receiver'.\n"); continue; }
        break;
    }
    while (1) {
        printf("Email: ");
        fgets(u.email, 40, stdin); u.email[strcspn(u.email, "\n")] = 0;
        if (strlen(u.email) > 0) break;
        printf("[Warning] Email cannot be empty!\n");
    }
    users[*count] = u; (*count)++;
    saveUsers(users, *count);
    printf("\n[Success] User '%s' registered successfully as '%s'!\n", u.username, u.role);
    return 1;
}

int loginUser(User *users, int count, char *loggedInUser) {
    char u[30], p[30];
    printf("Username: "); fgets(u, 30, stdin); u[strcspn(u, "\n")] = 0;
    printf("Password: "); fgets(p, 30, stdin); p[strcspn(p, "\n")] = 0;
    for (int i = 0; i < count; i++)
        if (strcmp(users[i].username, u) == 0 && strcmp(users[i].password, p) == 0) {
            strcpy(loggedInUser, u);
            printf("\n[Success] Welcome back, %s!\n", users[i].fullname);
            return 1;
        }
    printf("[Error] Invalid username or password!\n");
    return 0;
}

void displayAllUsers(const User *users, int count) {
    if (count == 0) { printf("\nNo users registered yet.\n"); return; }
    printf("\n%-5s %-20s %-30s %-10s %-25s\n", "#", "Username", "Full Name", "Role", "Email");
    printf("----------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < count; i++)
        printf("%-5d %-20s %-30s %-10s %-25s\n", i + 1, users[i].username, users[i].fullname, users[i].role, users[i].email);
}

int updateUserProfile(User *users, int count, const char *username) {
    for (int i = 0; i < count; i++)
        if (strcmp(users[i].username, username) == 0) {
            char in[50];
            printf("Current full name: %s\n", users[i].fullname);
            printf("New full name (Enter to skip): ");
            fgets(in, 50, stdin);
            if (in[0] != '\n') { in[strcspn(in, "\n")] = 0; strcpy(users[i].fullname, in); }
            printf("Current email: %s\n", users[i].email);
            printf("New email (Enter to skip): ");
            fgets(in, 40, stdin);
            if (in[0] != '\n') { in[strcspn(in, "\n")] = 0; strcpy(users[i].email, in); }
            saveUsers(users, count);
            printf("\n[Success] Profile updated!\n");
            return 1;
        }
    printf("[Error] User not found.\n");
    return 0;
}

void userMenu(char *loggedInUser, int *loggedIn) {
    int choice;
    while (1) {
        system(CLEAR_SCREEN);
        printf("\n");
        printCenteredLine('=', 36);
        printCentered("USER MANAGEMENT");
        printCenteredLine('=', 36);
        printf("                    1. View All Users\n");
        printf("                    2. Update My Profile\n");
        printf("                    3. Back to Main Menu\n");
        printCenteredLine('=', 36);
        printf("                    Choice: ");
        scanf("%d", &choice); getchar();
        switch (choice) {
            case 1:
                system(CLEAR_SCREEN);
                loadUsers(users, &userCount);
                displayAllUsers(users, userCount);
                printf("\n                    Press Enter to continue..."); getchar();
                break;
            case 2:
                system(CLEAR_SCREEN);
                printf("\n--- Update Profile ---\n");
                updateUserProfile(users, userCount, loggedInUser);
                printf("\n                    Press Enter to continue..."); getchar();
                break;
            case 3: return;
            default: printf("\n"); printCentered("Invalid choice.");
        }
    }
}