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

	printf("htons(PORT) = %d\n", htons(PORT));

	struct sockaddr_in cli_addr;
	int clen;

	fd_set set;

	// AF_INET - открываемый сокет должен быть сетевым.
	// SOCK_STREAM - требование, чтобы сокет был потоковым.
	// 0 - протокол выбирается по умолчанию.
	int sock = socket(AF_INET, SOCK_STREAM, 0);

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
	listen(sock, 1);

	// accept() устанавливает соединение в ответ на запрос клиента и создает
	// копию сокета для того, чтобы исходный сокет мог продолжать прослушивание.
	// Сервер перенаправляет запрошенное соединение на другой сокет (newsock), оставляя
	// сокет sock свободным для прослушивания запросов на установку соединения.
	FD_ZERO(&set);
	FD_SET(sock, &set);
	struct timeval timeout = {9, 0}; // 15 sec
	while (1)
	{
		int retval = select(sock + 1, &set, NULL, NULL, NULL);

		// if (retval)
		if (FD_ISSET(sock, &set))
		{
			printf("Данные доступны.\n");
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

			// char buf[10];
			// int count = read(retval, buf, sizeof(buf));
			// buf[count] = '\0';
			// printf("%s", buf);
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