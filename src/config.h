#ifndef CONFIG_H
#define CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
    #include <windows.h>
    #include <direct.h>
    #define MKDIR(path) _mkdir(path)
    #define CLEAR_SCREEN "cls"
    #define SLEEP(s) Sleep((s)*1000)
#else
    #include <unistd.h>
    #include <sys/stat.h>
    #define MKDIR(path) mkdir(path, 0755)
    #define CLEAR_SCREEN "clear"
    #define SLEEP(s) sleep(s)
#endif

#ifndef MAX_PATH
    #define MAX_PATH 260
#endif

#define MAX_USERS     100
#define MAX_PLATES    200
#define MAX_REQUESTS  500
#define MAX_MSG       200
#define MAX_LINE      512

typedef struct {
    char username[30];
    char password[30];
    char fullname[50];
    char role[10];
    char email[40];
} User;

typedef struct {
    int id;
    char donor[30];
    char foodName[40];
    char description[100];
    int quantity;
    char expiryDate[11];
    char status[15];
} Plate;

typedef struct {
    int id;
    int plateId;
    char requester[30];
    char status[15];
    char timestamp[20];
} Request;

typedef struct {
    char sender[30];
    char message[MAX_MSG];
    char timestamp[20];
} ChatMessage;

typedef struct {
    char dataFolder[MAX_PATH];
    int maxUsers;
    int maxPlates;
    int maxRequests;
} Config;

extern Config appConfig;
extern User users[MAX_USERS];
extern int userCount;
extern Plate plates[MAX_PLATES];
extern int plateCount;
extern Request requests[MAX_REQUESTS];
extern int requestCount;

extern char g_loggedInUser[30];
extern int g_loggedIn;

void getCurrentTimestamp(char *buffer, size_t size);

#endif