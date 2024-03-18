#ifndef USAGE_H
#define USAGE_H

#include "structs.h"

void runBuiltInCommand(Chain *chain);
// void runCommand(Command *command);
int runCommand(Command *command, int pipeIn[2], int pipeOut[2], int hasInput, int hasOutput, int input, int output);
void runChain(Chain *chain);
void freeError();

#endif