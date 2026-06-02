/**
 * @file request.h
 * @brief Request and transaction management interface
 * @author Member 4
 */

#ifndef REQUEST_H
#define REQUEST_H

#include "config.h"
#include "plate.h"

int createRequest(Request *requests, int *reqCount, Plate *plates, int plateCount, const char *requester);
void viewRequestsForMyPlates(const Request *requests, int reqCount, const Plate *plates, int plateCount, const char *donor);
void filterMyRequests(const Request *requests, int reqCount, const Plate *plates, int plateCount, const char *donor);
int acceptOrDeclineRequest(int reqId, Request *requests, int reqCount, Plate *plates, int plateCount);
void viewTransactionHistory(const Request *requests, int reqCount, const char *username);
void loadRequests(Request *requests, int *count);
void saveRequests(const Request *requests, int count);
void requestMenu(const char *loggedInUser);

#endif