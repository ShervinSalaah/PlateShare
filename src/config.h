/**
 * @file config.h
 * @brief Master configuration and shared definitions for PlateShare Pro
 * @author Team PlateShare
 * 
 * This header defines all structures, constants, and global declarations
 * used across the entire application. Every module includes this file.
 * 
 * Structures Defined:
 * - User:      Stores account information for registered users
 * - Plate:     Stores food donation details including pickup/delivery options
 * - Request:   Links a receiver to a requested plate with status tracking
 * - ChatMessage: Used for displaying chat messages in memory
 * - Config:    Application settings (data folder path, maximum limits)
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

#define MAX_USERS     100    /* Maximum number of registered users */
#define MAX_PLATES    200    /* Maximum number of food plates */
#define MAX_REQUESTS  500    /* Maximum number of requests */
#define MAX_MSG       200    /* Maximum length of a chat message */
#define MAX_LINE      512    /* Maximum length of a file line */
#define MAX_ADDRESS   150    /* Maximum length of pickup address */

/* ========== Structure Definitions ========== */

/**
 * @struct User
 * @brief Represents a registered user of the system
 * 
 * Stores account credentials and profile information.
 * Passwords are stored in plain text for this demonstration.
 */
typedef struct {
    char username[30];       /**< Unique login name */
    char password[30];       /**< User password (plain text for demo) */
    char fullname[50];       /**< Display name shown to other users */
    char email[40];          /**< Contact email address */
} User;

/**
 * @struct Plate
 * @brief Represents a food plate offered by a donor
 * 
 * Contains all details about a food donation including
 * pickup/delivery preferences and location.
 */
typedef struct {
    int id;                  /**< Unique plate identifier (auto-incremented) */
    char donor[30];          /**< Username of the person donating */
    char foodName[40];       /**< Name of the food item (required) */
    char description[100];   /**< Additional details (can be empty) */
    int quantity;            /**< Number of portions available */
    char expiryDate[11];     /**< Expiry date in YYYY-MM-DD format */
    char status[15];         /**< "Available", "Reserved", or "Donated" */
    char dateAdded[20];      /**< Timestamp when the plate was added */
    char pickupOption[20];   /**< "Self Pickup" or "Open to Delivery" */
    char address[MAX_ADDRESS]; /**< Pickup address or delivery area */
} Plate;

/**
 * @struct Request
 * @brief Represents a request for a food plate
 * 
 * Links a receiver to a specific plate and tracks the
 * status through the donation workflow.
 */
typedef struct {
    int id;                  /**< Unique request identifier */
    int plateId;             /**< ID of the requested plate (links to Plate) */
    char requester[30];      /**< Username of the person making the request */
    char status[15];         /**< "Pending", "Accepted", or "Declined" */
    char timestamp[20];      /**< When the request was made */
} Request;

/**
 * @struct ChatMessage
 * @brief Represents a chat message (used for display in memory)
 */
typedef struct {
    char sender[30];         /**< Username of the sender */
    char message[MAX_MSG];   /**< The message content */
    char timestamp[20];      /**< When the message was sent */
} ChatMessage;

/**
 * @struct Config
 * @brief Application configuration settings
 * 
 * Stores runtime settings that can be changed through the
 * Settings menu and persist via config.ini.
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
 * @param buffer Character array to store the result (must be >= 20 chars)
 * @param size Size of the buffer
 * 
 * Writes the current timestamp in "YYYY-MM-DD HH:MM:SS" format.
 * Used by chat messages, requests, notifications, and plate tracking.
 */
void getCurrentTimestamp(char *buffer, size_t size);

#endif