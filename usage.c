#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include "usage.h"

extern int *status;
extern char *currentPath;
extern ActiveOperator activeOperator;
extern ActiveOperator futureOperator;
extern void finalizeParser();

void printColor(char *color, char *msg) {
    #if EXT_PROMPT
    fprintf(stdout, "%s%s\x1b[0m", color, msg);
    #else
    fprintf(stdout, "%s", msg);
    #endif
}

void printPrompt() {
    #if EXT_PROMPT
    if (futureOperator == AO_NEWLINE) {
        fprintf(stdout, "%s> ", currentPath);
    }
    #endif
}

// handle built-in commands
void runBuiltInCommand(Chain *chain) {
    Command *command = chain->BuiltInCommand;
    switch (command->builtInCommand) {
        case BIC_EXIT:
            int exitStatus = 0;
            if (command->commandArgs->numArgs > 0) {
                exitStatus = command->commandArgs->args[0] != NULL ? atoi(command->commandArgs->args[0]) : 0; // exit with the argument if it exists
            }
            freeChain(chain);
            finalizeParser();
            exit(exitStatus);
        case BIC_STATUS:
            if (status != NULL) {
                fprintf(stdout ,"The most recent exit code is: %d\n", *status);
            } else {
                fprintf(stdout, "The most recent exit code is: 0\n");
            }
            break;
        case BIC_CD:
            if (command->commandArgs->numArgs > 0) {
                if (chdir(command->commandArgs->args[0]) != 0) {
                    printColor("\033[0;31m", "Error: cd directory not found!\n");
                    *status = 2;
                } else {
                    getcwd(currentPath, 1024 * sizeof(char));
                    *status = 0;
                }
            } else {
                printColor("\033[0;31m", "Error: cd requires folder to navigate to!\n");
                *status = 2;
            }
            break;
    }
}

// handle running commands
// void runCommand(Command *command) {
//     // check active operator to see if command should be run
//     if (activeOperator == AO_AND_STATEMENT) { // TODO IN FUTURE ASSIGNMENT
//         printPrompt();
//         freeCommand(command);
//         return;
//     }
//     // for && don't run if the previous command failed
//     if (activeOperator == AO_AND_OPERATOR && status != NULL && *status != 0) {
//         printPrompt();
//         freeCommand(command);
//         return;
//     }
//     // for || don't run if the previous command succeeded
//     if (activeOperator == AO_OR_OPERATOR && status != NULL && *status == 0) {
//         printPrompt();
//         freeCommand(command);
//         return;
//     }
//     if (command->builtInCommand != BIC_NONE) {
//         builtInCommandHandler(command);
//         printPrompt();
//         freeCommand(command);
//         return;
//     }
//     if (status == NULL) {
//         status = malloc(sizeof(int));
//     }

//     pid_t pid = fork();

//     if (pid < 0) {
//         printColor("\033[0;31m", "Error: fork() could not create a child process!\n");
//         freeCommand(command);
//         exit(EXIT_SUCCESS); /* EXIT_SUCCESS because we use Themis */
//     } else if (pid == 0) {
//         // child code
//         execvp(command->commandName, command->commandArgs->args);
//         printColor("\033[0;31m", "Error: command not found!\n");
//         freeCommand(command);
//         exit(127);
//     } else {
//         // parent code
//         wait(status);
//         if (WIFEXITED(*status)) {
//             *status = WEXITSTATUS(*status); // get the exit status in regular format
//         }
//         printPrompt();
//         freeCommand(command);
//     }
// }

// int runCommand(Command *command, int input, int ouput) {
//     pid_t pid = fork();
//     if (pid < 0) {
//         printColor("\033[0;31m", "Error: fork() could not create a child process!\n");
//         freeCommand(command);
//         exit(EXIT_SUCCESS); /* EXIT_SUCCESS because we use Themis */
//     } else if (pid == 0) {
//         // child code
//         if (input != STDIN_FILENO) {
//             dup2(input, STDIN_FILENO);
//             close(input);
//         }
//         if (ouput != STDOUT_FILENO) {
//             dup2(ouput, STDOUT_FILENO);
//             close(ouput);
//         }
//         execvp(command->commandName, command->commandArgs->args);
//         printColor("\033[0;31m", "Error: command not found!\n");
//         freeCommand(command);
//         exit(127);
//     }
//     return pid;
// }

int runCommand(Command *command, int pipeIn[2], int pipeOut[2], int hasInput, int hasOutput, int input, int output) {
    pid_t pid = fork();
    if (pid < 0) {
        printColor("\033[0;31m", "Error: fork() could not create a child process!\n");
        freeCommand(command);
        exit(EXIT_SUCCESS); /* EXIT_SUCCESS because we use Themis */
    } else if (pid == 0) {
        // child code
        if (hasInput) {
            close(pipeIn[1]);
            dup2(pipeIn[0], STDIN_FILENO);
            if (pipeIn[0] != STDIN_FILENO) {
                dup2(pipeIn[0], STDIN_FILENO);
                close(pipeIn[0]);
            }
        } else {
            if (input != STDIN_FILENO) {
                dup2(input, STDIN_FILENO);
                close(input);
            }
        }
        if (hasOutput) {
            close(pipeOut[0]);
            if (pipeOut[1] != STDOUT_FILENO) {
                dup2(pipeOut[1], STDOUT_FILENO);
                close(pipeOut[1]);
            }
        } else {
            if (output != STDOUT_FILENO) {
                dup2(output, STDOUT_FILENO);
                close(output);
            }
        }
        execvp(command->commandName, command->commandArgs->args);
        printColor("\033[0;31m", "Error: command not found!\n");
        freeCommand(command);
        exit(127);
    }
    return pid;
}

// handle running chains
void runChain(Chain *chain) {
    if (activeOperator == AO_AND_STATEMENT) { // TODO IN FUTURE ASSIGNMENT
        printPrompt();
        freeChain(chain);
        return;
    }
    // for && don't run if the previous chain failed
    if (activeOperator == AO_AND_OPERATOR && status != NULL && *status != 0) {
        printPrompt();
        freeChain(chain);
        return;
    }
    // for || don't run if the previous chain succeeded
    if (activeOperator == AO_OR_OPERATOR && status != NULL && *status == 0) {
        printPrompt();
        freeChain(chain);
        return;
    }
    if (status == NULL) {
        status = malloc(sizeof(int));
        *status = 0;
    }
    // run the built-in command if it exists
    if (chain->BuiltInCommand != NULL) {
        runBuiltInCommand(chain);
        printPrompt();
        freeChain(chain);
        return;
    }
    char *inputFile = chain->pipelineRedirections->redirections->inputFile;
    char *outputFile = chain->pipelineRedirections->redirections->outputFile;
    if (inputFile != NULL && outputFile != NULL && strcmp(inputFile, outputFile) == 0) {
        printColor("\033[0;31m", "Error: input and output files cannot be equal!\n");
        printPrompt();
        freeChain(chain);
        *status = 2;
        return;
    }

    // run the pipeline if it exists
    int numCommands = chain->pipelineRedirections->pipeline->numCommands;
    int **pipeFiles = malloc((numCommands - 1) * sizeof(int *));
    int *ids = malloc(numCommands * sizeof(int));
    for (int i = 0; i < numCommands - 1; i++) {
        pipeFiles[i] = malloc(2 * sizeof(int));
        if (pipe(pipeFiles[i]) < 0) {
            printColor("\033[0;31m", "Error: pipe() could not be created!\n");
            freeChain(chain);
            exit(EXIT_SUCCESS); /* EXIT_SUCCESS because we use Themis */
        }
    }
    for (int i = 0; i < numCommands; i++) {
        Command *command = chain->pipelineRedirections->pipeline->commands[i];
        int pipeIn[2], pipeOut[2];
        int hasInput = i > 0;
        int hasOutput = i < (numCommands - 1);
        if (i < numCommands - 1) {
            pipeOut[0] = pipeFiles[i][0];
            pipeOut[1] = pipeFiles[i][1];
        }
        if (i > 0) {
            pipeIn[0] = pipeFiles[i-1][0];
            pipeIn[1] = pipeFiles[i-1][1];
        }
        int input = -1;
        int output = -1;
        if (i == 0) {
            if (inputFile != NULL) {
                input = open(inputFile, O_RDONLY);
                if (input < 0) {
                    printColor("\033[0;31m", "Error: input file not found!\n");
                    freeChain(chain);
                    exit(EXIT_SUCCESS); /* EXIT_SUCCESS because we use Themis */
                }
            } else {
                input = STDIN_FILENO;
            }
        }
        if (i == numCommands - 1) {
            if (outputFile != NULL) {
                output = open(outputFile, O_WRONLY | O_CREAT | O_TRUNC);
                if (output < 0) {
                    printColor("\033[0;31m", "Error: output file could not be created!\n");
                    freeChain(chain);
                    exit(EXIT_SUCCESS); /* EXIT_SUCCESS because we use Themis */
                }
            } else {
                output = STDOUT_FILENO;
            }
        }
        ids[i] = runCommand(command, pipeIn, pipeOut, hasInput, hasOutput, input, output);
        // printf("pid: %d, command: %s\n", ids[i], command->commandName);

        if (i > 0) {
            close(pipeFiles[i-1][0]);
            close(pipeFiles[i-1][1]);
        }
    }

    for (int i = 0; i < numCommands; i++) {
        waitpid(ids[i], status, 0);
        // printf("pid: %d, status: %d\n", ids[i], *status);
        if (WIFEXITED(*status)) {
            *status = WEXITSTATUS(*status); // get the exit status in regular format
        }
    }

    free(ids);

    for (int i = 0; i < numCommands - 1; i++) {
        free(pipeFiles[i]);
    }

    free(pipeFiles);

    printPrompt();
    freeChain(chain);
}