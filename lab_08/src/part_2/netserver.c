#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/select.h>

#define PORT 9877

#define MAX_COUNT_SOCK 10

// Структура для сетевого взаимодействия.
// struct sockaddr_in
// {
// 	short int sin_family;		 // Семейство адресов
// 	unsigned short int sin_port; // Номер порта (главное отличие от sockaddr)
// 	struct in_addr sin_addr;	 // IP-адрес хоста.
// 	unsigned char sin_zero[8];	 // Дополнение до размера структуры sockaddr
// };

// struct in_addr
// {
// 	unsigned long s_addr;
// };

int main(int argc, char *argv[])
{
	setbuf(stdout, NULL);

	int arr_sock[MAX_COUNT_SOCK] = {0};
	// Структура специально предназначенная
	// для хранения адресов в формате Интернета.
	struct sockaddr_in serv_addr =
		{
			.sin_family = AF_INET,
			// INADDR_ANY - зарегистрировать нашу программу-сервер
			// на всех адресах машини, на которой она выполняется.
			.sin_addr.s_addr = INADDR_ANY,
			// htons() переписывает двухбайтовое значение порта так,
			// чтобы порядок байтов соответствовал сетевому.
			.sin_port = htons(PORT)};

	int max_sock;

	printf("htons(PORT) = %d\n", htons(PORT));

	struct sockaddr_in cli_addr;
	int clen;

	fd_set set;

	// AF_INET - открываемый сокет должен быть сетевым.
	// SOCK_STREAM - требование, чтобы сокет был потоковым.
	// 0 - протокол выбирается по умолчанию.
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	max_sock = sock;

	if (socket < 0)
	{
		printf("socket() failed: %d\n", errno);
		return EXIT_FAILURE;
	}

	if (bind(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("bind() failed: %d\n", errno);
		return EXIT_FAILURE;
	}

	// listen переводит сервер в режим ожидания запроса на соединение.
	// Второй параметр - максимальное число соединений, которые сервер может обрабатывать одновременно.
	if (listen(sock, 1) < 0)
	{
		printf("listen() failed: %d\n", errno);
		return -1;
	}

	// accept() устанавливает соединение в ответ на запрос клиента и создает
	// копию сокета для того, чтобы исходный сокет мог продолжать прослушивание.
	// Сервер перенаправляет запрошенное соединение на другой сокет (newsock), оставляя
	// сокет sock свободным для прослушивания запросов на установку соединения.

	// struct timeval timeout = {9, 0}; // 9 sec
	while (1)
	{
		FD_ZERO(&set);
		FD_SET(sock, &set);

		max_sock = sock;
		for (int i = 0; i < MAX_COUNT_SOCK; i++)
		{
			if (arr_sock[i] > 0)
			{
				FD_SET(arr_sock[i], &set);
				max_sock = arr_sock[i] > max_sock ? arr_sock[i] : max_sock;
			}
		}

		// первый аргумент на единицу больше самого большого номера описателей из всех наборов.
		// При возврате из функции select наборы описателей модифицируются, чтобы показать, какие описатели фактически изменили свой статус.
		int retval = select(max_sock + 1, &set, NULL, NULL, NULL);
		if (retval < 0)
		{
			printf("select() failed: %d\n", errno);
			return -1;
		}

		// if (retval)
		// Проверка на новое подключение.
		if (FD_ISSET(sock, &set))
		{
			// Если sock остался в set, значит он изменил свой статус
			// (к нему кто-то подключился) и значит он ожидает обработки.
			printf("Данные доступны.\n");
			printf("Новое подключение.\n");

			int newsock = accept(sock, NULL, NULL); // (struct sockaddr *)&cli_addr, &clen);
			if (newsock < 0)
			{
				printf("accept() failed: %d\n", errno);
				return -1;
			}

			int flag = 1;
			for (int i = 0; i < MAX_COUNT_SOCK && flag; i++)
			{
				if (arr_sock[i] == 0)
				{
					arr_sock[i] = newsock;
					FD_SET(newsock, &set);
					// TODO: тут бы из структурки (struct sockaddr *)&cli_addr достать данные о нем...
					printf("Новое подключение к серверу! Клиент номер %d socket = %d", i, newsock);
					flag = 0;
				}
			}
			/*
			int newsock = accept(sock, NULL, NULL); // (struct sockaddr *)&cli_addr, &clen);
			if (newsock < 0)
			{
				printf("accept() failed: %d\n", errno);
				return EXIT_FAILURE;
			}

			char buf[16];

			read(newsock, buf, sizeof(buf));
			printf("%s", buf);

			fflush(stdout);

			close(newsock);
			*/

			// char buf[10];
			// int count = read(retval, buf, sizeof(buf));
			// buf[count] = '\0';
			// printf("%s", buf);
		}

		// Проверяем, послали ли клиенты сообщения.
		for (int i = 0; i < MAX_COUNT_SOCK; i++)
		{
			if (arr_sock[i] > 0 && FD_ISSET(arr_sock[i], &set))
			{
				// recvfrom - получить сообщение из сокета.
				// Возвращает кол-во принятых байт.
				// -1, если ошибка.

				char buf[16];

				int rv = recvfrom(arr_sock[i], buf, sizeof(buf), 0, NULL, NULL);
				printf("rv = %d", rv);
				if (rv == 0)
				{
					printf("Отключение от сервера!");
					close(arr_sock[i]);
					arr_sock[i] = 0;
				}
				else
				{
					printf("%s", buf);
				}
				// TODO: Тут квадрат числе выводить.
			}
		}
	}
	// int newsock = accept(sock, (struct sockaddr *)&cli_addr, &clen);
	// if (newsock < 0)
	// {
	// 	printf("accept() failed: %d\n", errno);
	// 	return EXIT_FAILURE;
	// }

	// char buf[16];

	// read(newsock, buf, sizeof(buf));
	// printf("%s", buf);

	// close(newsock);
	close(sock);

	return 0;
}