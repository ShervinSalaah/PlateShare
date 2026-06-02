/**
 * @file request.h
 * @brief Food request and transaction management interface
 * @author 254189P SALIYA J.F
 * 
 * This header declares all functions for the food request module.
 * Community members can browse available food shared by others and
 * make requests. Food donors can view incoming requests, filter them
 * by status, and accept or decline them. Every transaction is recorded
 * so members can track their request history.
 * 
 * Key Features:
 * - Browse available food and make a request
 * - View all requests made on your food (newest first)
 * - Filter requests by status: Pending, Accepted, or Declined
 * - Accept or decline requests with confirmation
 * - View personal transaction history
 * - Automatic notifications sent to both parties on every action
 * - Prevents requesting your own food
 * - Persistent storage in requests.csv
 * 
 * Workflow:
 * 1. Member browses available food (from plate module)
 * 2. Member selects a food item to request
 * 3. Request is created with "Pending" status
 * 4. Food donor receives a notification
 * 5. Donor accepts or declines the request
 * 6. Both parties receive confirmation notifications
 * 7. If accepted, the food status changes to "Donated"
 * 8. If declined, the food becomes "Available" again
 */

#ifndef REQUEST_H
#define REQUEST_H

#include "config.h"
#include "plate.h"

/* ========== Core Request Operations ========== */

/**
 * @brief Creates a new request for an available food plate
 * 
 * Displays only plates with "Available" status so the member
 * can choose which food to request. Validates that:
 * - The plate ID is a valid number
 * - The plate exists and is still available
 * - The member is not requesting their own food
 * 
 * On success, marks the plate as "Reserved" and sends a
 * notification to the food donor with the requester's name.
 * 
 * @param requests Pointer to the global requests array
 * @param reqCount Pointer to current request count (incremented on success)
 * @param plates Pointer to the global plates array (to check availability)
 * @param plateCount Current number of plates
 * @param requester Username of the member making the request
 * @return 1 if request was created successfully, 0 if cancelled or failed
 */
int createRequest(Request *requests, int *reqCount, Plate *plates, int plateCount, const char *requester);

/**
 * @brief Shows all requests made on food owned by the logged-in donor
 * 
 * Displays requests in reverse chronological order (newest first)
 * so the donor can quickly see the most recent activity. For each
 * request, shows the request ID, food name, requester's username,
 * current status, and timestamp.
 * 
 * @param requests Pointer to the global requests array
 * @param reqCount Current number of requests
 * @param plates Pointer to the global plates array (to get food names)
 * @param plateCount Current number of plates
 * @param donor Username of the food owner viewing their requests
 */
void viewRequestsForMyPlates(const Request *requests, int reqCount, const Plate *plates, int plateCount, const char *donor);

/**
 * @brief Interactive menu to filter requests by status
 * 
 * Allows the donor to view only:
 * - Pending requests (waiting for action)
 * - Accepted requests (already approved)
 * - Declined requests (already rejected)
 * - All requests (newest first)
 * 
 * Runs in a sub-menu loop until the user chooses to go back.
 * Helps donors manage large numbers of requests efficiently.
 * 
 * @param requests Pointer to the global requests array
 * @param reqCount Current number of requests
 * @param plates Pointer to the global plates array
 * @param plateCount Current number of plates
 * @param donor Username of the food owner
 */
void filterMyRequests(const Request *requests, int reqCount, const Plate *plates, int plateCount, const char *donor);

/**
 * @brief Accepts or declines a pending request with confirmation
 * 
 * Shows the request details and asks the donor to choose:
 * - Accept (a): Food is marked as "Donated", requester is notified
 * - Decline (d): Food returns to "Available", requester is notified
 * 
 * Both actions require a second confirmation ("Are you sure? y/n")
 * to prevent accidental clicks. Any other key cancels the operation.
 * 
 * @param reqId The ID of the request to process
 * @param requests Pointer to the global requests array
 * @param reqCount Current number of requests
 * @param plates Pointer to the global plates array (to update status)
 * @param plateCount Current number of plates
 * @return 1 if the request was processed, 0 if cancelled or not found
 */
int acceptOrDeclineRequest(int reqId, Request *requests, int reqCount, Plate *plates, int plateCount);

/**
 * @brief Shows the complete request history for a member
 * 
 * Displays all requests made by the given user in reverse
 * chronological order (most recent first). Shows the request ID,
 * plate ID, current status, and timestamp for each request.
 * 
 * This allows members to track all their food requests in one place,
 * regardless of whether they were accepted or declined.
 * 
 * @param requests Pointer to the global requests array
 * @param reqCount Current number of requests
 * @param username Username of the member viewing their history
 */
void viewTransactionHistory(const Request *requests, int reqCount, const char *username);

/* ========== File Persistence ========== */

/**
 * @brief Loads all requests from requests.csv into the in-memory array
 * 
 * Reads the CSV file line by line and populates the requests array.
 * Each line contains: id,plateId,requester,status,timestamp
 * 
 * If the file doesn't exist yet (first run, no requests made),
 * sets count to 0 so the system starts with an empty request list.
 * 
 * @param requests Pointer to the global requests array to populate
 * @param count Pointer to store the number of loaded requests
 */
void loadRequests(Request *requests, int *count);

/**
 * @brief Saves all requests from memory to requests.csv
 * 
 * Writes the complete requests array to the CSV file.
 * Called after every change: creating, accepting, or declining requests.
 * Creates the file automatically if it doesn't exist yet.
 * 
 * CSV Format: id,plateId,requester,status,timestamp
 * 
 * @param requests Pointer to the global requests array
 * @param count Current number of requests to save
 */
void saveRequests(const Request *requests, int count);

/* ========== Menu ========== */

/**
 * @brief Displays the food request sub-menu
 * 
 * Provides options to:
 * 1. Request available food (browse and select)
 * 2. See requests on your food (newest first)
 * 3. Filter your requests by status
 * 4. Accept or decline a pending request
 * 5. View your request history
 * 6. Go back to main menu
 * 
 * Runs in a loop until the user chooses to go back.
 * Reloads data from files before each operation to ensure
 * the latest information from all LAN users is shown.
 * 
 * @param loggedInUser Username of the currently signed-in member
 */
void requestMenu(const char *loggedInUser);

#endif