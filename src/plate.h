/**
 * @file plate.h
 * @brief Food sharing (plate) management interface
 * @author Member 3
 * 
 * Provides functions for adding food donations, viewing plates,
 * filtering and sorting, and managing personal plate listings.
 */

#ifndef PLATE_H
#define PLATE_H

#include "config.h"

int addPlate(Plate *plates, int *count, const char *donor);
void displayAllPlates(const Plate *plates, int count);
void displayMyPlates(const Plate *plates, int count, const char *donor);
int deleteMyPlate(Plate *plates, int *count, const char *donor);
int searchPlateByName(const Plate *plates, int count, const char *food);
int searchPlateByDonor(const Plate *plates, int count, const char *donor);
void sortPlatesByExpiry(Plate *plates, int count);
void sortPlatesByDonor(Plate *plates, int count);
void sortPlatesByStatus(Plate *plates, int count);
void filterAndSortPlates(Plate *plates, int count);
int validateDate(const char *date);
int isDateInPast(const char *date);
void loadPlates(Plate *plates, int *count);
void savePlates(const Plate *plates, int count);
void plateMenu(const char *loggedInUser);

#endif