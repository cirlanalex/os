#ifndef USAGE_H
#define USAGE_H

#include "structs.h"

void runBuiltInCommand(Chain *chain);
// void runCommand(Command *command);
void runCommand(Command *command, int input, int ouput);
void runChain(Chain *chain);

#endif