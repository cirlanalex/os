%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <unistd.h>
    #include "structs.h"
    #include "usage.h"

    void yyerror(char *msg);    /* forward declaration */
    extern int yylex(void);
    extern void initLexer();
    extern void finalizeLexer();
    extern void printColor(char *color, char *msg);
    extern void printPrompt();

    // remember the previous operator to be used
    ActiveOperator activeOperator = AO_NONE;
    // remember the future operator to be used
    ActiveOperator futureOperator = AO_NEWLINE;
    // remember the status of the last command
    int *status = NULL;
    // remember the current path
    char *currentPath = NULL;
%}

%token EXIT_KEYWORD AND_OP OR_OP SEMICOLON NEWLINE AND_STATEMENT OR_STATEMENT INPUT_REDIRECT OUTPUT_REDIRECT STATUS_KEYWORD CD_KEYWORD

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

inputline               : chain AND_STATEMENT { futureOperator = AO_AND_STATEMENT; runCommand($1); activeOperator = AO_AND_STATEMENT; } inputline
                        | chain AND_OP { futureOperator = AO_AND_OPERATOR; runCommand($1); activeOperator = AO_AND_OPERATOR; } inputline
                        | chain OR_OP { futureOperator = AO_OR_OPERATOR; runCommand($1); activeOperator = AO_OR_OPERATOR; } inputline
                        | chain SEMICOLON { futureOperator = AO_SEMICOLON; runCommand($1); activeOperator = AO_SEMICOLON; } inputline  // allow use of semicolon as a command separator
                        | chain NEWLINE { futureOperator = AO_NEWLINE; runCommand($1); activeOperator = AO_NEWLINE; } inputline      // allow use of newline as a command separator
                        | chain { futureOperator = AO_NONE; runCommand($1); activeOperator = AO_NONE; }
                        | SEMICOLON { futureOperator = AO_SEMICOLON; activeOperator = AO_SEMICOLON; } inputline    // inappropriate semicolon usage is not considered an error
                        | NEWLINE { futureOperator = AO_NEWLINE; activeOperator = AO_NEWLINE; } inputline        // inappropriate newline usage is not considered an error
                        | /* empty */ { futureOperator = AO_NONE; activeOperator = AO_NONE; }
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
                        | options EXIT_KEYWORD { $$ = addArg($1, strdup("exit")); }
                        | options STATUS_KEYWORD { $$ = addArg($1, strdup("status")); }
                        | options CD_KEYWORD { $$ = addArg($1, strdup("cd")); }
                        | /* empty */ { $$ = createArgs(); }

builtin                 : EXIT_KEYWORD { $$ = BIC_EXIT; }
                        | STATUS_KEYWORD { $$ = BIC_STATUS; }
                        | CD_KEYWORD { $$ = BIC_CD; }
                        ;

%%

/* All code after the second pair of %% is just plain C where you typically
 * write your main function and such. */

void finalizeParser() {
    if (status != NULL) {
        free(status);
    }
    if (currentPath != NULL) {
        free(currentPath);
    }
    finalizeLexer();
}

void yyerror (char *msg) {
    printColor("\033[0;31m", "Error: invalid syntax!\n");
    finalizeParser();
    exit(EXIT_SUCCESS);  /* EXIT_SUCCESS because we use Themis */
}

int main() {
    // Initialize program
    initLexer();

    // Get current path
    currentPath = malloc(1024 * sizeof(char));

    getcwd(currentPath, 1024 * sizeof(char));

    printPrompt();

    // Start parsing process
    yyparse();

    // Cleanup
    finalizeParser();

    return EXIT_SUCCESS;
}
