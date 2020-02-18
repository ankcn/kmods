#include <sys/socket.h>
#include <linux/netlink.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "netlink.h"


int main(int argc, char* argv[])
{
	// Создание netlink сокета
	int sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
	if (sock_fd < 0) {
		printf("Error creating socket.\n");
		return -1;
	}

	// Адрес источника netlink сокета
	struct sockaddr_nl	src_addr = {
		.nl_family = AF_NETLINK,
		.nl_pid = getpid() // Идентификатор данного процесса
	};

	bind(sock_fd, (struct sockaddr*) &src_addr, sizeof(src_addr));

	// Адрес назначения netlink сокета
	struct sockaddr_nl	dest_addr = {
		.nl_family = AF_NETLINK,
		.nl_pid = 0, // Адресат - ядро
		.nl_groups = 0 // Юникастовая отправка
	};

	// Буфер под netlink сообщение, включая заголовок
	char buf[MAX_PAYLOAD] = "                Hello via netlink socket!";

	// Указатель на заголовок netlink сообщения
	struct nlmsghdr* nlh = (struct nlmsghdr*) buf;
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = getpid();

	// Указатель на текстовую часть сообщения (полезная нагрузка)
	char* message = buf + NLMSG_HDRLEN;

	// Заполняем сообщение переданными программе параметрами
	char* tmp = message;
	for (int i = 1; i < argc; ++i) {
		while (*argv[i])
			*tmp++ = *argv[i]++;
		*tmp++ = ' ';
	}
	if (tmp > message)
		*tmp = 0;

	struct iovec iov = {
		.iov_base = (void *) nlh,
		.iov_len = nlh->nlmsg_len
	};

	struct msghdr msg = {
		.msg_name = (void *) &dest_addr,
		.msg_namelen = sizeof(dest_addr),
		.msg_iov = &iov,
		.msg_iovlen = 1
	};

	// Отправка netlink сообщения ядру
	printf("Sending message to kernel\n");
	sendmsg(sock_fd, &msg, 0);

	// Получение сообщения от ядра
	printf("Waiting for message from kernel\n");
	recvmsg(sock_fd, &msg, 0);
	printf("Received message payload:\n%s\n", message);

	// Закрытие сокета
	close(sock_fd);

	return 0;
}
