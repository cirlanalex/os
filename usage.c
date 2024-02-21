#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include "usage.h"

extern int *status;
extern char *currentPath;
extern ActiveOperator activeOperator;
extern ActiveOperator futureOperator;
extern void finalizeParser();

void printColor(char *color, char *msg) {
    fprintf(stdout, "%s%s\x1b[0m", color, msg);
}

void printPrompt() {
    if (futureOperator == AO_NEWLINE) {
        fprintf(stdout, "%s> ", currentPath);
    }
}

// handle built-in commands
void builtInCommandHandler(Command *command) {
    switch (command->builtInCommand) {
        case BIC_EXIT:
            int exitStatus = 0;
            if (command->commandArgs->numArgs > 0) {
                exitStatus = command->commandArgs->args[0] != NULL ? atoi(command->commandArgs->args[0]) : 0; // exit with the argument if it exists
            }
            freeCommand(command);
            finalizeParser();
            exit(exitStatus);
        case BIC_STATUS:
            if (status != NULL) {
                fprintf(stdout ,"The most recent exit code is: %d\n", *status);
            } else {
                fprintf(stdout, "The most recent exit code is: 0\n");
            }
            break;
    }
}

// handle running commands
void runCommand(Command *command) {
    // check active operator to see if command should be run
    if (activeOperator == AO_AND_STATEMENT) { // TODO IN FUTURE ASSIGNMENT
        printPrompt();
        freeCommand(command);
        return;
    }
    // for && don't run if the previous command failed
    if (activeOperator == AO_AND_OPERATOR && status != NULL && *status != 0) {
        printPrompt();
        freeCommand(command);
        return;
    }
    // for || don't run if the previous command succeeded
    if (activeOperator == AO_OR_OPERATOR && status != NULL && *status == 0) {
        printPrompt();
        freeCommand(command);
        return;
    }
    if (command->builtInCommand != BIC_NONE) {
        builtInCommandHandler(command);
        printPrompt();
        freeCommand(command);
        return;
    }
    if (status == NULL) {
        status = malloc(sizeof(int));
    }

    pid_t pid = fork();

    if (pid < 0) {
        printColor("\033[0;31m", "fork() could not create a child process!\n");
        freeCommand(command);
        exit(EXIT_SUCCESS); /* EXIT_SUCCESS because we use Themis */
    } else if (pid == 0) {
        // child code
        execvp(command->commandName, command->commandArgs->args);
        printColor("\033[0;31m", "Error: command not found!\n");
        freeCommand(command);
        exit(127);
    } else {
        // parent code
        wait(status);
        if (WIFEXITED(*status)) {
            *status = WEXITSTATUS(*status); // get the exit status in regular format
        }
        printPrompt();
        freeCommand(command);
    }
}