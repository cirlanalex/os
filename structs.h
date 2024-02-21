#ifndef STRUCTS_H
#define STRUCTS_H

typedef enum BuiltInCommand {
    BIC_NONE,
    BIC_EXIT,
    BIC_STATUS 
} BuiltInCommand;

typedef struct Args {
    char **args;
    int numArgs;
} Args;

typedef struct Command {
    char *commandName;
    Args *commandArgs;
    BuiltInCommand builtInCommand;
} Command;

typedef enum ActiveOperator {
    AO_NONE,
    AO_AND_STATEMENT,
    AO_AND_OPERATOR,
    AO_OR_OPERATOR,
    AO_SEMICOLON,
    AO_NEWLINE
} ActiveOperator;

Args *createArgs();
Args *addArg(Args *args, char *arg);
void freeArgs(Args *args);

Command *createCommand(char *commandName, Args *commandArgs);
Command *createBuiltInCommand(BuiltInCommand builtInCommand, Args *commandArgs);
void freeCommand(Command *command);

#endif