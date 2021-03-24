#ifndef _READ_H_

#define _READ_H_

#include <stdio.h>

#include "constants.h"

void ReadFile(char fileName[MAX_LEN_CATALOG],  FILE * f_out, void (*myPrint)(char* , FILE *));

#endif // _READ_H_