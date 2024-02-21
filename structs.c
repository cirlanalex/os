#include <stdio.h>
#include <stdlib.h>

#include "structs.h"

// create an empty list of arguments
Args *createArgs() {
    Args *args = malloc(sizeof(Args));
    args->args = malloc(sizeof(char *));
    args->args[0] = NULL; // Space for the command name
    args->numArgs = 1;
    return args;
}

// add an argument to the list of arguments
Args *addArg(Args *args, char *arg) {
    args->args = realloc(args->args, (args->numArgs + 1) * sizeof(char *));
    args->args[args->numArgs] = arg;
    args->numArgs++;
    return args;
}

// free the list of arguments
void freeArgs(Args *args) {
    for (int i = 0; i < args->numArgs; i++) {
        free(args->args[i]);
    }
    free(args->args);
    free(args);
}

// create a command
Command *createCommand(char *commandName, Args *commandArgs) {
    Command *command = malloc(sizeof(Command));
    command->commandName = commandName;
    command->commandArgs = commandArgs;
    command->commandArgs->args[0] = commandName;                        // Set the first argument to the command name
    command->commandArgs->args = realloc(command->commandArgs->args, (command->commandArgs->numArgs + 1) * sizeof(char *));
    command->commandArgs->args[command->commandArgs->numArgs] = NULL;   // Null-terminate the array of arguments
    command->builtInCommand = BIC_NONE;
    return command;
}

// create a built-in command
Command *createBuiltInCommand(BuiltInCommand builtInCommand, Args *commandArgs) {
    Command *command = malloc(sizeof(Command));
    command->commandName = NULL;
    for (int i = 0; i < commandArgs->numArgs - 1; i++) {
        commandArgs->args[i] = commandArgs->args[i + 1];
    }
    commandArgs->numArgs--;
    commandArgs->args[commandArgs->numArgs] = NULL;
    commandArgs->args = realloc(commandArgs->args, commandArgs->numArgs * sizeof(char *));
    command->commandArgs = commandArgs;
    command->builtInCommand = builtInCommand;
    return command;
}

// free a command
void freeCommand(Command *command) {
    // the name is freed when the args are freed
    freeArgs(command->commandArgs);
    free(command);
}