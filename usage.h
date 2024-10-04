#ifndef USAGE_H
#define USAGE_H

#include "structs.h"

void runChain(Chain *chain);
void freeError();
void sigIntHandler(int signo);

#endif