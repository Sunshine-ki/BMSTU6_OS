#include <dirent.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>

#include "path.h"
#include "errors.h"
#include "constants.h"

void catalog(char *pathname)
{
	printf("Begin:\n%s\n", pathname);
	Path *path = CreatePath();
	char *curr_pathname;
	int curr_len;

	DIR *dir;
	struct dirent *entry;
	struct stat statbuf;

	// Записываем в statbuf информацию о файле.
	if (lstat(pathname, &statbuf) == ERROR_LSTAT)
		error_lstat();

	if (!S_ISDIR(statbuf.st_mode))
		err_sys("Введенный путь не является каталогом."); // Не каталог.

	Push(path, pathname);

	// int len_dir = len(path->path)

	int len_tmp = 50;
	char tmp[len_tmp];
	for (int i = 0; i < 50; i++)
		tmp[i] = '*';
	
	tmp[len_tmp] = '\0';

	len_tmp = 2;
	tmp[len_tmp] = '\0';
	// tmp[len_tmp] = '*';


	// len_tmp = 25;
	// tmp[len_tmp] = '\0';
	// printf("%s \n", tmp);

	while (path->count)
	{		
		curr_pathname = Pop(path);

		dir = opendir(curr_pathname);
		if (!dir)
			err_sys("Ошибка diropen");

		curr_len = strlen(curr_pathname);
		while ((entry = readdir(dir)) != NULL)
		{
			tmp[len_tmp] = '*';
			len_tmp += 2;
			tmp[len_tmp] = '\0';

			// Пропускаем ка­та­ло­ги "." и ".."
			if (strcmp(entry->d_name, ".") == 0 ||
				strcmp(entry->d_name, "..") == 0)
				continue;
			// printf("current_file = %s\n", entry->d_name);

			curr_pathname = (char *)realloc(curr_pathname, curr_len + strlen(entry->d_name) + 2); // +2 == +1 +1 == +'/' +'\0'
			curr_pathname[curr_len] = '/';
			strcpy(&curr_pathname[curr_len + 1], entry->d_name);

			// printf("current_file = %s\n", curr_pathname);
			printf("%s %s \n", tmp, entry->d_name);
			if (lstat(curr_pathname, &statbuf) == ERROR_LSTAT)
				error_lstat(); // TODO: Передавать параметром путь.

			if (S_ISDIR(statbuf.st_mode)) // Каталог.
			{
				// printf("Это каталог\n");
				Push(path, curr_pathname);
			}

		tmp[len_tmp] = '*';
		len_tmp -= 2;
		tmp[len_tmp] = '\0';

			// printf("Catalog = %d\n", S_ISDIR());
			// printf("%ld - %s [%d] %d\n\n", entry->d_ino, entry->d_name, entry->d_type, entry->d_reclen);
		}
		// printf("LEN = %d", path->count);


		if (closedir(dir) < 0)
			exit(1);
	}



	DestroyPath(path);
}

// void catalog(char *pathname)
// {
// 	printf("%s\n", pathname);
// 	int count = 0;

// 	char **array_dir = malloc(sizeof(char *));
// 	int len_dir = sizeof(char) * (strlen(pathname) + 1);
// 	int len_curr_dir = len_dir;
// 	array_dir[0] = malloc(len_dir);
// 	char *current_file = malloc(sizeof(char) * (strlen(pathname) + 1));
// 	strcpy(current_file, pathname);

// 	count++;

// 	strcpy(array_dir[0], pathname);
// 	printf("Result copy = %s\n\n", array_dir[0]);

// 	struct stat statbuf;
// 	// struct dirent *dirp;
// 	// DIR *dp;

// 	// Записываем в statbuf информацию о файле.
// 	if (lstat(pathname, &statbuf) == ERROR_LSTAT)
// 		error_lstat();

// 	// st_mode - режим доступа
// 	if (!S_ISDIR(statbuf.st_mode)) // Не каталог.
// 		err_sys("Введенный путь не является каталогом.");

// 	DIR *dir;
// 	struct dirent *entry;

// 	dir = opendir(pathname);
// 	if (!dir)
// 		err_sys("Ошибка diropen");

// 	// TODO:
// 	// while(count) если это каталог, то добваляем его в массив каталогов и увеличиваем count++
// 	// Иначе что-то делаем с файлом (можем просто вывести полный путь до него и информацию о нем).
// 	// while (count)
// 	// {
// 	// while ((entry = readdir(dir)) != NULL)
// 	// {
// 	// 	// Пропускаем ка­та­ло­ги "." и ".."
// 	// 	if (strcmp(entry->d_name, ".") == 0 ||
// 	// 		strcmp(entry->d_name, "..") == 0)
// 	// 		continue;

// 	// 	len_curr_dir += strlen(entry->d_name);
// 	// 	current_file = (char *)realloc(current_file, len_curr_dir);
// 	// 	current_file[len_dir-1] = '/';
// 	// 	strcpy(&current_file[len_dir], entry->d_name);

// 	// 	if (lstat(current_file, &statbuf) == ERROR_LSTAT)
// 	// 		error_lstat(); // TODO: Передавать параметром путь.

// 	// 	printf("current_file = %s\n", current_file);

// 	// 	if (S_ISDIR(statbuf.st_mode)) // Каталог.
// 	// 	{
// 	// 		printf("Это каталог\n");

// 	// 		count++;
// 	// 		array_dir = (char *)realloc(array_dir, sizeof(char *) * count);
// 	// 		array_dir[count-1] = malloc(len_curr_dir);

// 	// 	}
// 	// 	// if (lstat(pathname, &statbuf) == ERROR_LSTAT)
// 	// 	// error_lstat();
// 	// 	// printf("Catalog = %d\n", S_ISDIR());
// 	// 	printf("%ld - %s [%d] %d\n\n", entry->d_ino, entry->d_name, entry->d_type, entry->d_reclen);
// 	// }
// 	// // }

// 	// for (int i = 0; i < count; i++)
// 	// 	free(array_dir[i]);
// 	// free(array_dir);

// 	closedir(dir);
// }
