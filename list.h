#ifndef LIST_H
#define LIST_H

#include <unistd.h>

// structure for a node in the list
typedef struct BackgroundProcess {
    pid_t id;
    int pid;
    struct BackgroundProcess *next;
} BackgroundProcess;

// structure for the list
typedef struct BackgroundList {
    BackgroundProcess *head;
    BackgroundProcess *tail;
    int lastId;
} BackgroundList;

BackgroundList *createBackgroundList();
void addBackgroundProcess(BackgroundList *list, pid_t pid);
void removeBackgroundProcessByPID(BackgroundList *list, pid_t pid);
void removeBackgroundProcessByID(BackgroundList *list, int id);
void printBackgroundList(BackgroundList *list);
int isEmptyBackgroundList(BackgroundList *list);
void freeBackgroundList(BackgroundList *list);

#endif