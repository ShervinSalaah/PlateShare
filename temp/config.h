#ifndef CONFIG_H
#define CONFIG_H

/* Standard libraries used across the project */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Platform-specific definitions */
#ifdef _WIN32
    #include <windows.h>          /* for Sleep() */
    #define CLEAR_SCREEN "cls"
#else
    #include <unistd.h>           /* for sleep() */
    #define CLEAR_SCREEN "clear"
#endif

/* ---------- Maximum sizes ---------- */
#define MAX_USERS     100
#define MAX_PLATES    200
#define MAX_REQUESTS  500
#define MAX_PATH      256
#define MAX_MSG       200
#define MAX_LINE      512

/* ---------- 5 Structure Definitions ---------- */

typedef struct {
    char username[30];
    char password[30];
    char fullname[50];
    char role[10];               /* "donor" or "receiver" */
    char email[40];
} User;

typedef struct {
    int id;
    char donor[30];
    char foodName[40];
    char description[100];
    int quantity;
    char expiryDate[11];         /* YYYY-MM-DD */
    char status[15];             /* "Available", "Reserved", "Donated" */
} Plate;

typedef struct {
    int id;
    int plateId;
    char requester[30];
    char status[15];             /* "Pending", "Accepted", "Declined" */
    char timestamp[20];
} Request;

typedef struct {
    char sender[30];
    char message[MAX_MSG];
    char timestamp[20];
} ChatMessage;                   /* used only for display, not persistent storage */

typedef struct {
    char dataFolder[MAX_PATH];
    int maxUsers;
    int maxPlates;
    int maxRequests;
} Config;

/* ---------- Global variables (declared extern, defined in main.c) ---------- */
extern Config appConfig;
extern User users[MAX_USERS];
extern int userCount;
extern Plate plates[MAX_PLATES];
extern int plateCount;
extern Request requests[MAX_REQUESTS];
extern int requestCount;

/* ---------- Shared utility function (implemented in core.c) ---------- */
void getCurrentTimestamp(char *buffer, size_t size);

#endif