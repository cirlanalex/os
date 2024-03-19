#include <stdio.h>
#include <stdlib.h>

#include "structs.h"

extern Chain *lastChain;
extern Pipeline *lastPipeline;
extern Redirections *lastRedirections;
extern Command *lastCommand;
extern Args *lastArgs;

extern void skipLine();

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

// create a file list
FileList *createFileList() {
    FileList *fileList = malloc(sizeof(FileList));
    fileList->files = malloc(sizeof(char *));
    fileList->files[0] = NULL;
    fileList->numFiles = 0;
    return fileList;
}

// add a file to a file list
FileList *addFile(FileList *fileList, char *file) {
    fileList->files = realloc(fileList->files, (fileList->numFiles + 1) * sizeof(char *));
    fileList->files[fileList->numFiles] = file;
    fileList->numFiles++;
    return fileList;
}

// free a file list
void freeFileList(FileList *fileList) {
    for (int i = 0; i < fileList->numFiles; i++) {
        free(fileList->files[i]);
    }
    free(fileList->files);
    free(fileList);
}

// create redirections
Redirections *createRedirections() {
    Redirections *redirections = malloc(sizeof(Redirections));
    redirections->inputFiles = createFileList();
    redirections->outputFiles = createFileList();
    redirections->errorFiles = createFileList();
    // remember the last redirections
    lastRedirections = redirections;
    return redirections;
}

// add an input file to redirections
Redirections *addRedirection(Redirections *redirections, char *file, RedirectionType type) {
    if (type == R_INPUT) {
        #if EXT_PROMPT
        #else
        if (redirections->inputFiles->numFiles > 0) {
            return NULL;
        }
        #endif
        redirections->inputFiles = addFile(redirections->inputFiles, file);
        return redirections;
    } 
    if (type == R_OUTPUT) {
        #if EXT_PROMPT
        #else
        if (redirections->outputFiles->numFiles > 0) {
            return NULL;
        }
        #endif
        redirections->outputFiles = addFile(redirections->outputFiles, file);
        return redirections;
    }
    redirections->errorFiles = addFile(redirections->errorFiles, file);
    return redirections;
}

// free redirections
void freeRedirections(Redirections *redirections) {
    freeFileList(redirections->inputFiles);
    freeFileList(redirections->outputFiles);
    freeFileList(redirections->errorFiles);
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