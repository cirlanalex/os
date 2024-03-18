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
    extern void freeError();

    Chain *lastChain = NULL;
    Pipeline *lastPipeline = NULL;
    Redirections *lastRedirections = NULL;
    Command *lastCommand = NULL;
    Args *lastArgs = NULL;
    void freeError();


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
%type <pipeline> pipeline
%type <redirections> redirections
%type <chain> chain

%union {
    Chain *chain;
    BuiltInCommand builtInCommand;
    Pipeline *pipeline;
    Redirections *redirections;
    Command *executableCommand;
    Args *args;
    char *stringValue;
}

%%

input                   : inputline NEWLINE input
                        | error NEWLINE { freeError(); yyerrok; } input
                        | /* empty */

inputline               : chain AND_STATEMENT { futureOperator = AO_AND_STATEMENT; runChain($1); activeOperator = AO_AND_STATEMENT; lastChain = NULL; } inputline
                        | chain AND_OP { futureOperator = AO_AND_OPERATOR; runChain($1); activeOperator = AO_AND_OPERATOR; lastChain = NULL; } inputline
                        | chain OR_OP { futureOperator = AO_OR_OPERATOR; runChain($1); activeOperator = AO_OR_OPERATOR; lastChain = NULL; } inputline
                        | chain SEMICOLON { futureOperator = AO_SEMICOLON; runChain($1); activeOperator = AO_SEMICOLON; lastChain = NULL; } inputline  // allow use of semicolon as a command separator
                        | chain { futureOperator = AO_NONE; runChain($1); activeOperator = AO_NONE; lastChain = NULL; }
                        | SEMICOLON { futureOperator = AO_SEMICOLON; activeOperator = AO_SEMICOLON; } inputline    // inappropriate semicolon usage is not considered an error
                        | /* empty */ { futureOperator = AO_NONE; activeOperator = AO_NONE; }
                        ;

chain                   : pipeline redirections { $$ = createChain(createPipelineRedirections($1, $2), NULL); }
                        | builtin options { $$ = createChain(NULL, createBuiltInCommand($1, $2)); }
                        ;

redirections            : INPUT_REDIRECT WORD OUTPUT_REDIRECT WORD { $$ = createRedirections($2, $4); }
                        | OUTPUT_REDIRECT WORD INPUT_REDIRECT WORD { $$ = createRedirections($4, $2); }
                        | OUTPUT_REDIRECT WORD { $$ = createRedirections(NULL, $2); }
                        | INPUT_REDIRECT WORD { $$ = createRedirections($2, NULL); }
                        | /* empty */ { $$ = createRedirections(NULL, NULL); }
                        ;

pipeline                : pipeline OR_STATEMENT command { $$ = addCommandToPipeline($1, $3); }
                        | command { $$ = createPipeline($1); }
                        ;

command                 : WORD options { $$ = createCommand($1, $2); }
                        ;

options                 : options STRING { $$ = addArg($1, $2);}
                        | options WORD { $$ = addArg($1, $2); }
                        | options EXIT_KEYWORD { $$ = addArg($1, strdup("exit")); }
                        | options STATUS_KEYWORD { $$ = addArg($1, strdup("status")); }
                        | options CD_KEYWORD { $$ = addArg($1, strdup("cd")); }
                        | /* empty */ { $$ = createArgs(); lastArgs = $$; }

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
    // "finalizeParser"();
    // exit(EXIT_SUCCESS);  /* EXIT_SUCCESS because we use Themis */
}

int main() {
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
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
