#include <stdio.h>

#include "read.h"

void ReadFile(char fileName[MAX_LEN_CATALOG],  FILE * f_out, void (*myPrint)(char* , FILE *))
{
	char buf[BUF_SIZE];
	int len;

	FILE *f = fopen(fileName, FILE_READ);

	// Возвращает кол-во действительно прочитанных объектов.
	while ((len = fread(buf, 1, BUF_SIZE, f)) > 0)
	{	
		for (int i = 0; i < len; i++)
			if (buf[i] == 0)
				buf[i] = 10; 	// EOF
		buf[len] = 0; 			// '\0'
		myPrint(buf, f_out);
	}

	fclose(f);
}