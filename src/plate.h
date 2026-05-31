#ifndef PLATE_H 
#define PLATE_H 
#include "config.h" 
 
int addPlate(Plate *plates, int *count, const char *donor); 
void displayAllPlates(const Plate *plates, int count); 
int searchPlateByName(const Plate *plates, int count, const char *food); 
void sortPlatesByExpiry(Plate *plates, int count); 
int validateDate(const char *date); 
void loadPlates(Plate *plates, int *count); 
void savePlates(const Plate *plates, int count); 
 
#endif 
