/**
 * @file plate.h
 * @brief Food sharing (plate) management interface
 * @author 254186E RUBASINHEGE S.N
 * 
 * This header declares all functions for the food sharing module.
 * Community members can add food they want to share, view available
 * food from others, search and filter listings, and manage their
 * own donations. The module handles food expiry tracking, pickup
 * and delivery options, and sorting by various criteria.
 * 
 * Key Features:
 * - Add new food with name, quantity, expiry date, and pickup method
 * - View all available food or only your own listings
 * - Search by food name or donor name
 * - Sort by expiry date, donor name, or pickup method
 * - Delete your own food (only if still available)
 * - Validate dates to prevent past/expired food
 * - Persistent storage in plates.csv
 */

#ifndef PLATE_H
#define PLATE_H

#include "config.h"

/* ========== Core Plate Operations ========== */

/**
 * @brief Adds a new food plate to the system
 * 
 * Walks the user through entering food details including:
 * - Food name (required, cannot be empty)
 * - Description (optional, can skip with Enter)
 * - Quantity (required, must be a positive number)
 * - Expiry date (required, must be valid future date)
 * - Pickup option (Self Pickup or Open to Delivery)
 * - Address or delivery area (required)
 * 
 * Automatically sends notifications to all community members
 * when new food is shared.
 * 
 * @param plates Pointer to the global plates array
 * @param count Pointer to current plate count (incremented on success)
 * @param donor Username of the person sharing the food
 * @return 1 if plate was added successfully, 0 if failed or cancelled
 */
int addPlate(Plate *plates, int *count, const char *donor);

/**
 * @brief Displays all food plates in the system
 * 
 * Shows a formatted table with ID, donor, food name, quantity,
 * expiry date, pickup method, and status for every plate.
 * 
 * @param plates Pointer to the global plates array
 * @param count Current number of plates
 */
void displayAllPlates(const Plate *plates, int count);

/**
 * @brief Displays only the plates donated by a specific user
 * 
 * Filters the plates array to show only those where the donor
 * matches the given username. Useful for members who want to
 * manage their own food listings.
 * 
 * @param plates Pointer to the global plates array
 * @param count Current number of plates
 * @param donor Username of the donor whose plates to show
 */
void displayMyPlates(const Plate *plates, int count, const char *donor);

/**
 * @brief Deletes one of the user's own food plates
 * 
 * Shows the user's plates first, then asks for the ID to delete.
 * Only plates with "Available" status can be deleted. Plates that
 * are already "Reserved" or "Donated" cannot be removed.
 * 
 * @param plates Pointer to the global plates array
 * @param count Pointer to current plate count (decremented on success)
 * @param donor Username of the person trying to delete
 * @return 1 if plate was deleted, 0 if cancelled or not allowed
 */
int deleteMyPlate(Plate *plates, int *count, const char *donor);

/* ========== Search Functions ========== */

/**
 * @brief Searches for plates by food name (case-insensitive)
 * 
 * Performs a substring search on food names. For example,
 * searching "rice" will find "Fried Rice" and "Rice Pudding".
 * 
 * @param plates Pointer to the global plates array
 * @param count Current number of plates
 * @param food The search term to look for in food names
 * @return Number of matching plates found
 */
int searchPlateByName(const Plate *plates, int count, const char *food);

/**
 * @brief Searches for plates by donor username (case-insensitive)
 * 
 * Performs a substring search on donor names. Useful for finding
 * all food shared by a particular community member.
 * 
 * @param plates Pointer to the global plates array
 * @param count Current number of plates
 * @param donor The search term to look for in donor names
 * @return Number of matching plates found
 */
int searchPlateByDonor(const Plate *plates, int count, const char *donor);

/* ========== Sort Functions ========== */

/**
 * @brief Sorts plates by expiry date (oldest first)
 * 
 * Uses bubble sort to arrange plates so that food expiring
 * soonest appears at the top. This helps community members
 * prioritise food that needs to be eaten quickly.
 * 
 * @param plates Pointer to the global plates array
 * @param count Current number of plates
 */
void sortPlatesByExpiry(Plate *plates, int count);

/**
 * @brief Sorts plates alphabetically by donor name (A-Z)
 * 
 * Groups all plates from the same donor together. Useful
 * for seeing what each community member is sharing.
 * 
 * @param plates Pointer to the global plates array
 * @param count Current number of plates
 */
void sortPlatesByDonor(Plate *plates, int count);

/**
 * @brief Sorts plates by status (Available, Reserved, Donated)
 * 
 * Groups plates by their current status so you can easily
 * see which food is still available versus already taken.
 * 
 * @param plates Pointer to the global plates array
 * @param count Current number of plates
 */
void sortPlatesByStatus(Plate *plates, int count);

/**
 * @brief Interactive menu for filtering and sorting plates
 * 
 * Provides options to:
 * - Sort by expiry date (oldest first)
 * - Sort by donor name (A-Z)
 * - Sort by pickup method
 * - Search by food name
 * - Search by donor name
 * 
 * Runs in a sub-menu loop until the user chooses to go back.
 * 
 * @param plates Pointer to the global plates array
 * @param count Current number of plates
 */
void filterAndSortPlates(Plate *plates, int count);

/* ========== Validation Functions ========== */

/**
 * @brief Checks if a date string is in valid YYYY-MM-DD format
 * 
 * Validates that:
 * - The string is exactly 10 characters
 * - Dashes are in the correct positions
 * - Year is 2024 or later
 * - Month is between 1 and 12
 * - Day is valid for the given month
 * 
 * @param date The date string to validate
 * @return 1 if the date format is valid, 0 otherwise
 */
int validateDate(const char *date);

/**
 * @brief Checks if a date has already passed (is before today)
 * 
 * Compares the given date with the current system date.
 * Used to prevent users from sharing food with an expiry
 * date that has already passed.
 * 
 * @param date The date string to check (must be valid YYYY-MM-DD)
 * @return 1 if the date is in the past, 0 if today or future
 */
int isDateInPast(const char *date);

/* ========== File Persistence ========== */

/**
 * @brief Loads all plates from plates.csv into the in-memory array
 * 
 * Reads the CSV file line by line and populates the plates array.
 * Handles both the current 10-field format and older 7-field format
 * for backward compatibility with data from earlier versions.
 * If the file doesn't exist, sets count to 0.
 * 
 * CSV Format: id,donor,foodName,description,quantity,expiryDate,
 *             dateAdded,pickupOption,address,status
 * 
 * @param plates Pointer to the global plates array to populate
 * @param count Pointer to store the number of loaded plates
 */
void loadPlates(Plate *plates, int *count);

/**
 * @brief Saves all plates from memory to plates.csv
 * 
 * Writes the complete plates array to the CSV file.
 * Called after every change (adding, deleting, status changes).
 * Creates the file if it doesn't exist yet.
 * 
 * @param plates Pointer to the global plates array
 * @param count Current number of plates to save
 */
void savePlates(const Plate *plates, int count);

/* ========== Menu ========== */

/**
 * @brief Displays the food sharing sub-menu
 * 
 * Provides options to:
 * 1. Share new food
 * 2. See all food
 * 3. See my food
 * 4. Remove my food
 * 5. Find and sort food
 * 6. Go back to main menu
 * 
 * Runs in a loop until the user chooses to go back.
 * 
 * @param loggedInUser Username of the currently signed-in member
 */
void plateMenu(const char *loggedInUser);

#endif