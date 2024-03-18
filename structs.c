#include <stdio.h>
#include <stdlib.h>

#include "structs.h"

extern Chain *lastChain;
extern Pipeline *lastPipeline;
extern Redirections *lastRedirections;
extern Command *lastCommand;
extern Args *lastArgs;

// create an empty list of arguments
Args *createArgs() {
    Args *args = malloc(sizeof(Args));
    args->args = malloc(sizeof(char *));
    args->args[0] = NULL; // Space for the command name
    args->numArgs = 1;
    // remember the last arguments
    lastArgs = args;
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
    // remember the last command
    lastCommand = command;
    // forget the unnecessary data
    lastArgs = NULL;
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
    // forget the unnecessary data
    lastArgs = NULL;
    return command;
}

// free a command
void freeCommand(Command *command) {
    // the name is freed when the args are freed
    freeArgs(command->commandArgs);
    free(command);
}

// create a pipeline
Pipeline *createPipeline(Command *command) {
    Pipeline *pipeline = malloc(sizeof(Pipeline));
    pipeline->commands = malloc(sizeof(Command *));
    pipeline->commands[0] = command;
    pipeline->numCommands = 1;
    // remember the last pipeline
    lastPipeline = pipeline;
    // forget the unnecessary data
    lastCommand = NULL;
    return pipeline;
}

// add a command to a pipeline
Pipeline *addCommandToPipeline(Pipeline *pipeline, Command *command) {
    pipeline->commands = realloc(pipeline->commands, (pipeline->numCommands + 1) * sizeof(Command *));
    pipeline->commands[pipeline->numCommands] = command;
    pipeline->numCommands++;
    // forget the unnecessary data
    lastCommand = NULL;
    return pipeline;
}

// free a pipeline
void freePipeline(Pipeline *pipeline) {
    for (int i = 0; i < pipeline->numCommands; i++) {
        freeCommand(pipeline->commands[i]);
    }
    free(pipeline->commands);
    free(pipeline);
}

// create redirections
Redirections *createRedirections(char *inputFile, char *outputFile) {
    Redirections *redirections = malloc(sizeof(Redirections));
    redirections->inputFile = inputFile;
    redirections->outputFile = outputFile;
    // remember the last redirections
    lastRedirections = redirections;
    return redirections;
}

// free redirections
void freeRedirections(Redirections *redirections) {
    if (redirections->inputFile != NULL) {
        free(redirections->inputFile);
    }
    if (redirections->outputFile != NULL) {
        free(redirections->outputFile);
    }
    free(redirections);
}

// create a pipeline redirections
PipelineRedirections *createPipelineRedirections(Pipeline *pipeline, Redirections *redirections) {
    PipelineRedirections *pipelineRedirections = malloc(sizeof(PipelineRedirections));
    pipelineRedirections->pipeline = pipeline;
    pipelineRedirections->redirections = redirections;
    // forget the unnecessary data
    lastPipeline = NULL;
    lastRedirections = NULL;
    return pipelineRedirections;
}

// free a pipeline redirections
void freePipelineRedirections(PipelineRedirections *pipelineRedirections) {
    freeRedirections(pipelineRedirections->redirections);
    freePipeline(pipelineRedirections->pipeline);
    free(pipelineRedirections);
}

// create a chain
Chain *createChain(PipelineRedirections *pipelineRedirections, Command *BuiltInCommand) {
    Chain *chain = malloc(sizeof(Chain));
    chain->pipelineRedirections = pipelineRedirections;
    chain->BuiltInCommand = BuiltInCommand;
    // remember the last chain
    lastChain = chain;
    return chain;
}

// free a chain
void freeChain(Chain *chain) {
    if (chain->pipelineRedirections != NULL) {
        freePipelineRedirections(chain->pipelineRedirections);
    }
    if (chain->BuiltInCommand != NULL) {
        freeCommand(chain->BuiltInCommand);
    }
    free(chain);
}