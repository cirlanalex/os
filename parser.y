%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include <sys/wait.h>
    #include "structs.h"

    void yyerror(char *msg);    /* forward declaration */
    extern int yylex(void);
    extern void initLexer();
    extern void finalizeLexer();

    // remember the previous operator to be used
    ActiveOperator activeOperator = AO_NONE;
    // remember the status of the last command
    int *status = NULL;
    void runCommand(Command *command);
%}

%token EXIT_KEYWORD AND_OP OR_OP SEMICOLON NEWLINE AND_STATEMENT OR_STATEMENT INPUT_REDIRECT OUTPUT_REDIRECT STATUS_KEYWORD

%token <stringValue> STRING
%token <stringValue> WORD

%type <builtInCommand> builtin
%type <args> options
%type <executableCommand> command
%type <executableCommand> pipeline    // TO CHANGE IN FUTURE ASSIGNMENT
%type <executableCommand> chain       // TO CHANGE IN FUTURE ASSIGNMENT

%union {
    char *stringValue;
    Command *executableCommand;
    Args *args;
    BuiltInCommand builtInCommand;
}

%%

start                   : inputline { free($1); }
                        ;

inputline               : chain AND_STATEMENT { runCommand($1); activeOperator = AO_AND_STATEMENT; } inputline
                        | chain AND_OP { runCommand($1); activeOperator = AO_AND_OPERATOR; } inputline
                        | chain OR_OP { runCommand($1); activeOperator = AO_OR_OPERATOR; } inputline
                        | chain SEMICOLON { runCommand($1); activeOperator = AO_SEMICOLON; } inputline
                        | chain NEWLINE { runCommand($1); activeOperator = AO_NEWLINE; } inputline 
                        | chain { runCommand($1); activeOperator = AO_NONE; }
                        | /* empty */ { activeOperator = AO_NONE; }
                        ;

chain                   : pipeline redirections { $$ = $1; } // redirections are not implemented yet
                        | builtin options { $$ = createBuiltInCommand($1, $2); }
                        ;

redirections            : INPUT_REDIRECT WORD OUTPUT_REDIRECT WORD { free($2); free($4); }  // TODO IN FUTURE ASSIGNMENT
                        | OUTPUT_REDIRECT WORD INPUT_REDIRECT WORD { free($2); free($4); }  // TODO IN FUTURE ASSIGNMENT
                        | OUTPUT_REDIRECT WORD { free($2); }                                // TODO IN FUTURE ASSIGNMENT
                        | INPUT_REDIRECT WORD { free($2); }                                 // TODO IN FUTURE ASSIGNMENT
                        | /* empty */
                        ;

pipeline                : command OR_STATEMENT pipeline { $$ = $1; }                        // TODO IN FUTURE ASSIGNMENT
                        | command { $$ = $1; }
                        ;

command                 : WORD options { $$ = createCommand($1, $2); }
                        ;

options                 : options STRING { $$ = addArg($1, $2);}
                        | options WORD { $$ = addArg($1, $2); }
                        | /* empty */ { $$ = createArgs(); }

builtin                 : EXIT_KEYWORD { $$ = BIC_EXIT; }
                        | STATUS_KEYWORD { $$ = BIC_STATUS; }
                        ;

%%

/* All code after the second pair of %% is just plain C where you typically
 * write your main function and such. */

void finalizeParser() {
    if (status != NULL) {
        free(status);
    }
    finalizeLexer();
}

void yyerror (char *msg) {
    fprintf(stdout, "Error: invalid syntax!\n");
    // finalizeParser();
    // exit(EXIT_SUCCESS);  /* EXIT_SUCCESS because we use Themis */
}

void builtInCommandHandler(Command *command) {
    switch (command->builtInCommand) {
        case BIC_EXIT:
            int exitStatus = 0;
            if (command->commandArgs->numArgs > 0) {
                exitStatus = command->commandArgs->args[0] != NULL ? atoi(command->commandArgs->args[0]) : 0;
            }
            freeCommand(command);
            finalizeParser();
            exit(exitStatus);
        case BIC_STATUS:
            if (status != NULL) {
                printf("The most recent exit code is: %d\n", *status);
            } else {
                printf("The most recent exit code is: 0\n");
            }
            break;
    }
}

void runCommand(Command *command) {
    // check active operator to see if command should be run
    if (activeOperator == AO_AND_STATEMENT) { // TODO IN FUTURE ASSIGNMENT
        freeCommand(command);
        return;
    }
    // for && don't run if the previous command failed
    if (activeOperator == AO_AND_OPERATOR && status != NULL && *status != 0) {
        freeCommand(command);
        return;
    }
    // for || don't run if the previous command succeeded
    if (activeOperator == AO_OR_OPERATOR && status != NULL && *status == 0) {
        freeCommand(command);
        return;
    }
    if (command->builtInCommand != BIC_NONE) {
        builtInCommandHandler(command);
        freeCommand(command);
        return;
    }
    if (status == NULL) {
        status = malloc(sizeof(int));
    }

    pid_t pid = fork();

    if (pid < 0) {
        fprintf(stderr, "fork() could not create a child process!\n");
        freeCommand(command);
        exit(EXIT_SUCCESS); /* EXIT_SUCCESS because we use Themis */
    } else if (pid == 0) {
        // child code
        execvp(command->commandName, command->commandArgs->args);
        fprintf(stdout, "Error: command not found!\n");
        freeCommand(command);
        exit(127);
    } else {
        // parent code
        wait(status);
        if (WIFEXITED(*status)) {
            *status = WEXITSTATUS(*status); // get the exit status in regular format
        }
        freeCommand(command);
    }
}

int main() {
    // Initialize program
    initLexer();

    // Start parsing process
    yyparse();

    // Cleanup
    finalizeParser();

    return EXIT_SUCCESS;
}
