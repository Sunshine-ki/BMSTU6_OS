#ifndef _PATH_H_

#define _PATH_H_

typedef struct
{
	int count;
	char **path;
} Path;

Path *CreatePath();

void DestroyPath(Path *path);

void Push(Path *path, char *elem);

char *Pop(Path *path);

void Output(const Path const * const path);

#endif