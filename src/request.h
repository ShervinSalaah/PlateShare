#ifndef REQUEST_H 
#define REQUEST_H 
#include "config.h" 
#include "plate.h" 
 
int createRequest(Request *requests, int *reqCount, Plate *plates, int plateCount, const char *requester); 
void viewRequestsForMyPlates(const Request *requests, int reqCount, const Plate *plates, int plateCount, const char *donor); 
int acceptRequest(int reqId, Request *requests, int reqCount, Plate *plates, int plateCount); 
int declineRequest(int reqId, Request *requests, int reqCount, Plate *plates, int plateCount); 
void viewTransactionHistory(const Request *requests, int reqCount, const char *username); 
void loadRequests(Request *requests, int *count); 
void saveRequests(const Request *requests, int count); 
 
#endif 
