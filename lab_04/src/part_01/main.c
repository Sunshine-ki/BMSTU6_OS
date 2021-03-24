// proc - предоставляет различную системную информацию.
// https://www.opennet.ru/man.shtml?topic=proc&category=5&russian=0

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "print.h"
#include "read.h"

void WrapperForOutput(char catalog[MAX_LEN_CATALOG], char *curr_name, void (*myPrint)(char *, FILE *), FILE *f_out)
{
	int str_end_index = strlen(catalog);
	
	strcat(catalog, curr_name);
	// fprintf(f_out, BLUE "File: " YELLOW "%s" BLUE "\nContent:\n" GREEN, catalog);
	fprintf(f_out, GREEN "File: " YELLOW "%s" GREEN "\nContent:\n" BLUE, catalog);
	ReadFile(catalog, f_out, myPrint);
	
	catalog[str_end_index] = 0;
	fprintf(f_out, "\n\n");
}

// Аргументом командой строки передается pid.
int main(int argc, char *argv[])
{
	char catalog[MAX_LEN_CATALOG] = "/proc/";
	char curr_catalog[MAX_LEN_CATALOG];

	if (argc != COUNT_ARGS)
		strcat(catalog, "self/");
	else
		strcat((strcat(catalog, argv[PID])), "/");

	strcpy(curr_catalog, catalog);

	FILE *f_out = fopen(FILE_RESULT, FILE_WRITE);
	
	// STAT - файл, содержащий большой объем информации о процессе.
	WrapperForOutput(curr_catalog, "stat", PrintStat, f_out);

	// ENVIRON - файл, содержащий окружение процесса.
	WrapperForOutput(curr_catalog, "environ", SimplePrint, f_out);

	// CMDLINE - файл, указывает на директорию процесса. 
	WrapperForOutput(curr_catalog, "cmdline", SimplePrint, f_out);

	// MAPS - файл, содержащий информацию, о выделенных процессу участков памяти.
	WrapperForOutput(curr_catalog, "maps", SimplePrint, f_out);

	// // status
	// WrapperForOutput(curr_catalog, "status", SimplePrint, f_out);
	
	// STAM предоставляет информацию о состоянии памяти в страницах, как единицах измерения.
	WrapperForOutput(curr_catalog, "statm", PrintStatm, f_out);
	
	fclose(f_out);
	return OK;
}