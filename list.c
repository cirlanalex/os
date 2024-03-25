#include <stdio.h>
#include <stdlib.h>

#include "list.h"

// create a new list
BackgroundList *createBackgroundList() {
    BackgroundList *list = malloc(sizeof(BackgroundList));
    list->head = NULL;
    list->tail = NULL;
    list->lastId = 1;
    return list;
}

// add a new process to the list
void addBackgroundProcess(BackgroundList *list, pid_t pid) {
    BackgroundProcess *process = malloc(sizeof(BackgroundProcess));
    process->id = list->lastId++;
    process->pid = pid;
    process->next = NULL;
    if (list->head == NULL) {
        list->head = process;
    } else {
        list->tail->next = process;
    }
    list->tail = process;
}

// remove a process from the list by its PID
void removeBackgroundProcessByPID(BackgroundList *list, pid_t pid) {
    BackgroundProcess *previous = NULL;
    BackgroundProcess *current = list->head;
    while (current != NULL) {
        if (current->pid == pid) {
            if (previous == NULL) {
                list->head = current->next;
            } else {
                previous->next = current->next;
            }
            if (current == list->tail) {
                list->tail = previous;
            }
            free(current);
            return;
        }
        previous = current;
        current = current->next;
    }
}

// remove a process from the list by its ID
void removeBackgroundProcessByID(BackgroundList *list, int id) {
    BackgroundProcess *previous = NULL;
    BackgroundProcess *current = list->head;
    while (current != NULL) {
        if (current->id == id) {
            if (previous == NULL) {
                list->head = current->next;
            } else {
                previous->next = current->next;
            }
            if (current == list->tail) {
                list->tail = previous;
            }
            free(current);
            return;
        }
        previous = current;
        current = current->next;
    }
}

// get the PID of a process by its ID
pid_t getBackgroundProcessPID(BackgroundList *list, int id) {
    BackgroundProcess *current = list->head;
    while (current != NULL) {
        if (current->id == id) {
            return current->pid;
        }
        current = current->next;
    }
    return -1;
}

// print the list in reverse order
void printBackgroundList(BackgroundProcess *current) {
    if (current == NULL) {
        return;
    }
    printBackgroundList(current->next);
    printf("Process running with index %d\n", current->id);
}

// check if the list is empty
int isEmptyBackgroundList(BackgroundList *list) {
    return list->head == NULL;
}

// free the list
void freeBackgroundList(BackgroundList *list) {
    BackgroundProcess *current = list->head;
    while (current != NULL) {
        BackgroundProcess *next = current->next;
        free(current);
        current = next;
    }
    free(list);
}