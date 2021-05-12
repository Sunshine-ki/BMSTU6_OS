#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <errno.h>

#define PORT 9877

// struct hostent
// {
// 	char *h_name;		/* официальное имя машины */
// 	char **h_aliases;	/* список псевдонимов */
// 	int h_addrtype;		/* тип адреса машины */
// 	int h_length;		/* длина адреса */
// 	char **h_addr_list; /* список адресов */
// }
// #define h_addr h_addr_list[0] /* для совместимости с предыдущими версиями */

int main(int argc, char *argv[])
{
	struct sockaddr_in serv_addr =
		{
			.sin_family = AF_INET,
			// INADDR_ANY - все адреса локального хоста (0.0.0.0);
			.sin_addr.s_addr = INADDR_ANY,
			.sin_port = htons(PORT)};

	struct hostent *server;
	int sock;

	// server = gethostbyname(argv[1]);
	// if (server == NULL)
	// {
	// 	printf("Host not found\n");
	// 	return EXIT_FAILURE;
	// }

	sock = socket(AF_INET, SOCK_STREAM, 0);

	if (socket < 0)
	{
		perror("socket() failed");
		return EXIT_FAILURE;
	}
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("connect() failed: %d", errno);
		return EXIT_FAILURE;
	}
	// Устанавливаем соединение. После установки соединения
	// сокет готов к передаче и приему данных.
	for (int i = 0; i < 5; i++)
	{
		char buf[16];
		scanf("%s", buf);
		sendto(sock, buf, strlen(buf) + 1, 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

		printf("Send!");
		fflush(0);
	}

	close(sock);

	// По умолчанию созданный сокет блокирующий.
	// fcntl() + флаг O_NONBLOCK делает его неблокирующим.
	//
	// Сокеты бывают блокирующие и неблокирующие.
	// В случае блокирующих сокетов при попытке прочитать(и записать)
	// данные функция чтения будет ждать до тех пор, пока не прочитает
	// хотя бы один байт или произойдет разрыв соединения или придет сигнал.
	// В случае неблокирующих сокетов функция чтения проверяет,
	// есть ли данные в буфере, и если есть - сразу возвращает,
	// если нет, то она не ждет и также сразу возвращает, что прочитано 0 байт.
	//
	// Теперь любой вызов функции read() для сокета sock будет возвращать управление сразу же.
	// Если на входе сокета нет данных для чтения, функция read() вернет значение EAGAIN.
	// sock = socket(PF_INET, SOCK_STREAM, 0);
	// fcntl(sock, F_SETFL, O_NONBLOCK);

	return 0;
}