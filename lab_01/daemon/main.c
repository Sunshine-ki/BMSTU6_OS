// Переход в корень диска, необходим для того, чтобы
// впоследствии не было проблем связанных с размонтированием дисков.
// Если текущая папка демона будет находиться на диске,
// который необходимо будет отмонтировать, то система не даст этого,
// до тех пор, пока демон не будет остановлен.

#include "apue.h"
#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>

#include <pthread.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <sys/stat.h>

void reader(void)
{
	//
}

#define LOCKFILE "/var/run/daemon.pid"
#define LOCKMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)

// Установка блокировки для записи на весь файл
int lockfile(int fd)
{
	struct flock
	{
		short l_type;
		short l_whence;
		off_t l_start;
		off_t l_len;
		pid_t l_pid;
	} fl;

	// struct flock fl;
	fl.l_type = F_WRLCK;

	fl.l_start = 0;
	fl.l_whence = SEEK_SET;
	fl.l_len = 0;
	return (fcntl(fd, F_SETLK, &fl));
}

int already_running(void)
{
	int fd;
	char buf[16];

	fd = open(LOCKFILE, O_RDWR | O_CREAT, LOCKMODE);
	if (fd < 0)
	{
		syslog(LOG_ERR, "невозможно открыть %s: %s",
			   LOCKFILE, strerror(errno));
		exit(1);
	}
	if (lockfile(fd) < 0)
	{
		if (errno == EACCES || errno == EAGAIN)
		{
			close(fd);
			return (1);
		}
		syslog(LOG_ERR, "невозможно установить блокировку на %s: %s",
			   LOCKFILE, strerror(errno));
		exit(1);
	}
	ftruncate(fd, 0);
	sprintf(buf, "%ld", (long)getpid());
	write(fd, buf, strlen(buf) + 1);
	return (0);
}

void daemonize(const char *cmd)
{

	int i, fd0, fd1, fd2;
	pid_t pid;
	struct rlimit rl;
	struct sigaction sa;

	/*
         * Сбросить маску режима создания файла.
         */
	umask(0);
	/*
         * Получить максимально возможный номер дескриптора файла.
         */
	if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
	{
		exit(2);
		// err_quit("%s: невозможно получить максимальный номер дескриптора ", cmd);
	}

	/*
         * Стать лидером нового сеанса, чтобы утратить управляющий терминал.
         */
	if ((pid = fork()) < 0)
	{
		// err_quit("%s: ошибка вызова функции fork", cmd);
		exit(1);
	}
	else if (pid != 0) /* родительский процесс */
		exit(0);

	setsid();

	/*
         * Обеспечить невозможность обретения управляющего терминала в будущем.
         */
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGHUP, &sa, NULL) < 0)
	{
		// err_quit("%s: невозможно игнорировать сигнал SIGHUP", cmd);
		exit(3);
	}
	if ((pid = fork()) < 0)
	{
		// err_quit("%s: ошибка вызова функции fork", cmd);
		exit(4);
	}
	else if (pid != 0) /* родительский процесс */
		exit(0);

	FILE *fp = fopen("log.txt", "a");

	fprintf(fp, "Стадия 1\n");

	/*
         * Назначить корневой каталог текущим рабочим каталогом,
         * чтобы впоследствии можно было отмонтировать файловую систему.
         */
	if (chdir("/") < 0)
	{
		// err_quit("%s: невозможно сделать текущим рабочим каталогом /", cmd);
		exit(5);
	}

	fprintf(fp, "Стадия 2\n");
	fclose(fp);

	/*
         * Закрыть все открытые файловые дескрипторы.
         */
	if (rl.rlim_max == RLIM_INFINITY)
		rl.rlim_max = 1024;
	for (i = 0; i < rl.rlim_max; i++)
		close(i);

	/*
         * Присоединить файловые дескрипторы 0, 1 и 2 к /dev/null.
         */
	fd0 = open("/dev/null", O_RDWR);
	fd1 = dup(0);
	fd2 = dup(0);

	fp = fopen("log.txt", "a");

	fprintf(fp, "Стадия 3\n");

	fclose(fp);
	/*
         * Инициализировать файл журнала.
         */
	openlog(cmd, LOG_CONS, LOG_DAEMON);
	if (fd0 != 0 || fd1 != 1 || fd2 != 2)
	{
		syslog(LOG_ERR, "ошибочные файловые дескрипторы %d %d %d",
			   fd0, fd1, fd2);
		exit(1);
	}
}

int main(int argc, char *argv[])
{
	int err;
	pthread_t tid;
	char *cmd;
	struct sigaction sa;
	if ((cmd = strrchr(argv[0], '/')) == NULL)
		cmd = argv[0];
	else
		cmd++;

	daemonize(cmd);

	if (already_running())
	{
		printf("де­мон уже за­пу­щен");
		syslog(LOG_ERR, "де­мон уже за­пу­щен");
		exit(1);
	}

	return 0;
}
