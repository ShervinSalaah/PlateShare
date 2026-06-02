/**
 * @file config.h
 * @brief Master configuration and shared definitions for PlateShare Pro
 * @author Team PlateShare
 * 
 * This header defines all structures, constants, and global declarations
 * used across the entire application. Every module includes this file.
 */

#ifndef CONFIG_H
#define CONFIG_H

/* ========== Standard Libraries ========== */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ========== Platform-Specific Setup ========== */
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

/* ========== Constants ========== */
#ifndef MAX_PATH
    #define MAX_PATH 260
#endif

#define MAX_USERS     100
#define MAX_PLATES    200
#define MAX_REQUESTS  500
#define MAX_MSG       200
#define MAX_LINE      512

/* ========== Structure Definitions ========== */

/**
 * @struct User
 * @brief Represents a registered user of the system
 */
typedef struct {
    char username[30];   /**< Unique login name */
    char password[30];   /**< User password (plain text for demo) */
    char fullname[50];   /**< Display name */
    char role[10];       /**< "donor" or "receiver" */
    char email[40];      /**< Contact email address */
} User;

/**
 * @struct Plate
 * @brief Represents a food plate offered by a donor
 */
typedef struct {
    int id;              /**< Unique plate identifier */
    char donor[30];      /**< Username of the donor */
    char foodName[40];   /**< Name of the food item */
    char description[100]; /**< Detailed description */
    int quantity;        /**< Number of portions */
    char expiryDate[11]; /**< Expiry date in YYYY-MM-DD format */
    char status[15];     /**< "Available", "Reserved", or "Donated" */
    char dateAdded[20];  /**< Timestamp when the plate was added */
} Plate;

/**
 * @struct Request
 * @brief Represents a request for a food plate
 */
typedef struct {
    int id;              /**< Unique request identifier */
    int plateId;         /**< ID of the requested plate (foreign key) */
    char requester[30];  /**< Username of the person making the request */
    char status[15];     /**< "Pending", "Accepted", or "Declined" */
    char timestamp[20];  /**< When the request was made */
} Request;

/**
 * @struct ChatMessage
 * @brief Represents a chat message (used for display in memory)
 */
typedef struct {
    char sender[30];     /**< Username of the sender */
    char message[MAX_MSG]; /**< The message content */
    char timestamp[20];  /**< When the message was sent */
} ChatMessage;

/**
 * @struct Config
 * @brief Application configuration settings
 */
typedef struct {
    char dataFolder[MAX_PATH]; /**< Path where all data files are stored */
    int maxUsers;              /**< Maximum number of registered users */
    int maxPlates;             /**< Maximum number of plates */
    int maxRequests;           /**< Maximum number of requests */
} Config;

/* ========== Global Variable Declarations ========== */
extern Config appConfig;
extern User users[MAX_USERS];
extern int userCount;
extern Plate plates[MAX_PLATES];
extern int plateCount;
extern Request requests[MAX_REQUESTS];
extern int requestCount;

/* ========== Session State ========== */
extern char g_loggedInUser[30]; /**< Currently logged-in username */
extern int g_loggedIn;           /**< Login flag: 1=logged in, 0=not */

/* ========== Shared Utility Function ========== */
/**
 * @brief Gets the current date and time as a formatted string
 * @param buffer Character array to store the result
 * @param size Size of the buffer
 */
void getCurrentTimestamp(char *buffer, size_t size);

#endif