#ifndef STRUCTS_H
#define STRUCTS_H

// types of built-in commands
typedef enum BuiltInCommand {
    BIC_NONE,
    BIC_EXIT,
    BIC_STATUS,
    BIC_CD
} BuiltInCommand;

// structure for command arguments
typedef struct Args {
    char **args;
    int numArgs;
} Args;

// structure for command
typedef struct Command {
    char *commandName;
    Args *commandArgs;
    BuiltInCommand builtInCommand;
} Command;

// structure for pipeline
typedef struct Pipeline {
    Command **commands;
    int numCommands;
} Pipeline;

// types of operators
typedef enum ActiveOperator {
    AO_NONE,
    AO_AND_STATEMENT,
    AO_AND_OPERATOR,
    AO_OR_OPERATOR,
    AO_SEMICOLON,
    AO_NEWLINE
} ActiveOperator;

// types of redirections
typedef enum RedirectionType {
    R_INPUT,
    R_OUTPUT,
    R_ERROR
} RedirectionType;

// structure for file lists for redirections
typedef struct FileList {
    char **files;
    int numFiles;
} FileList;

// structure for redirections
typedef struct Redirections {
    FileList *inputFiles;
    FileList *outputFiles;
    FileList *errorFiles;
} Redirections;

// structure for pipeline redirections
typedef struct PipelineRedirections {
    Pipeline *pipeline;
    Redirections *redirections;
} PipelineRedirections;

// structure for chain
typedef struct Chain {
    PipelineRedirections *pipelineRedirections;
    Command *BuiltInCommand;
} Chain;

Args *createArgs();
Args *addArg(Args *args, char *arg);
void freeArgs(Args *args);

Command *createCommand(char *commandName, Args *commandArgs);
Command *createBuiltInCommand(BuiltInCommand builtInCommand, Args *commandArgs);
void freeCommand(Command *command);

Pipeline *createPipeline(Command *command);
Pipeline *addCommandToPipeline(Pipeline *pipeline, Command *command);
void freePipeline(Pipeline *pipeline);

FileList *createFileList();
FileList *addFile(FileList *fileList, char *file);
void freeFileList(FileList *fileList);

Redirections *createRedirections();
Redirections *addRedirection(Redirections *redirections, char *file, RedirectionType type);
void freeRedirections(Redirections *redirections);

PipelineRedirections *createPipelineRedirections(Pipeline *pipeline, Redirections *redirections);
void freePipelineRedirections(PipelineRedirections *pipelineRedirections);

Chain *createChain(PipelineRedirections *pipelineRedirections, Command *BuiltInCommand);
void freeChain(Chain *chain);

#endif