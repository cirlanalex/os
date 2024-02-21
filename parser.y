%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "structs.h"
    #include "usage.h"

    void yyerror(char *msg);    /* forward declaration */
    extern int yylex(void);
    extern void initLexer();
    extern void finalizeLexer();

    // remember the previous operator to be used
    ActiveOperator activeOperator = AO_NONE;
    // remember the status of the last command
    int *status = NULL;
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

inputline               : chain AND_STATEMENT { runCommand($1); activeOperator = AO_AND_STATEMENT; } inputline
                        | chain AND_OP { runCommand($1); activeOperator = AO_AND_OPERATOR; } inputline
                        | chain OR_OP { runCommand($1); activeOperator = AO_OR_OPERATOR; } inputline
                        | chain SEMICOLON { runCommand($1); activeOperator = AO_SEMICOLON; } inputline  // allow use of semicolon as a command separator
                        | chain NEWLINE { runCommand($1); activeOperator = AO_NEWLINE; } inputline      // allow use of newline as a command separator
                        | chain { runCommand($1); activeOperator = AO_NONE; }
                        | SEMICOLON { activeOperator = AO_SEMICOLON; } inputline    // inappropriate semicolon usage is not considered an error
                        | NEWLINE { activeOperator = AO_NEWLINE; } inputline        // inappropriate newline usage is not considered an error
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
                        | options EXIT_KEYWORD { $$ = addArg($1, strdup("exit")); }
                        | options STATUS_KEYWORD { $$ = addArg($1, strdup("status")); }
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
    finalizeParser();
    exit(EXIT_SUCCESS);  /* EXIT_SUCCESS because we use Themis */
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
