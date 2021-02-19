#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include "path.h"

Path *CreatePath()
{
	Path *res = (Path *)malloc(sizeof(Path));
	res->count = 0;
	res->path = NULL;

	return res;
}

void DestroyPath(Path *path)
{
	for (int i = 0; i < path->count; i++)
		if (path->path[i])
			free(path->path[i]);
	if (path->path)
		free(path->path);
	if (path)
		free(path);
}

void Push(Path *path, char *elem)
{
	(path->count)++;

	if (path->path)
		path->path = realloc(path->path, sizeof(char *) * path->count);
	else
		path->path = malloc(sizeof(char *));

	path->path[path->count - 1] = malloc(strlen(elem) + 1);
	strcpy(path->path[path->count - 1], elem);
}

char *Pop(Path *path)
{
	if (path->count < 1)
		return NULL;

	char *res = path->path[path->count - 1];
	// free(path->path[path->count - 1]);
	
	(path->count)--;
	path->path = realloc(path->path, sizeof(char *) * path->count);
	
	return res;
}

void Output(const Path const * const path)
{
	for (int i = 0; i < path->count; i++)
		printf("%s ", path->path[i]);
	puts("");
}

// int main(void)
// {
// 	Path *begin_path = CreatePath();


// 	Push(begin_path, "aaa");
// 	Push(begin_path, "bbb");
// 	Push(begin_path, "ccc");

// 	Output(begin_path);

// 	char *elem = Pop(begin_path);
// 	printf("elem = %s\n", elem);
// 	free(elem);

// 	Output(begin_path);

// 	DestroyPath(begin_path);

// 	return 0;
// }