/*
 * kping.c: модуль ядра для выполнения ping
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/ip.h>
#include <linux/icmp.h>
#include <linux/in.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrey Tarasov");
MODULE_DESCRIPTION("Test module for ICMP ping");


// Имя директории в sysfs
#define KOBJ_DIR_NAME	"myping"

// Имя атрибута/файла в sysfs
#define KOBJ_FILE_NAME	address

// Идентификатор ICMP эхо
#define PING_ID	10

// Размер буфера под IP пакет
#define PACKET_BUF_LEN	1500


// Вспомогательная структура для объекта sysfs
static struct kobject* kobjp;

// Сокет
static struct socket* sock = NULL;

// Адрес сокета
static struct sockaddr_in sa = {
	.sin_addr.s_addr = (1 << 24) + 127,
	.sin_family = AF_INET,
	.sin_port = 1
};

// Буфер для получения IP пакета
static char buf[PACKET_BUF_LEN];


/*
 * sysfs_read - Чтение в userspace объекта sysfs
 * Сообщается текущий IP адрес
 */
static ssize_t sysfs_read(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	printk(KERN_INFO "Reading IP address from sysfs file\n");
	return sprintf(buf, "IP address to ping to: %pI4\n", &sa.sin_addr.s_addr);
}

/*
 * send_icmp_request - Отправка ICMP эхо запроса
 */
static int send_icmp_request(void)
{
	static u8 seq = 0;
	struct icmphdr pingh = {
		.type = ICMP_ECHO,
		.checksum = ~ htons((ICMP_ECHO << 8) + PING_ID + seq),
		.un.echo.id = htons(PING_ID),
		.un.echo.sequence = htons(seq++)
	};
	struct kvec vec = {
		.iov_base = &pingh,
		.iov_len = sizeof(pingh)
	};
	struct msghdr msg = {
		.msg_name = (struct sockaddr*) &sa,
		.msg_namelen = sizeof(struct sockaddr),
		.msg_flags = 0,
		.msg_control = NULL,
		.msg_controllen = 0
	};
	return kernel_sendmsg(sock, &msg, &vec, 1, vec.iov_len);
}

/*
 * recv_icmp_reply - Получение ICMP эхо ответа
 */
static int recv_icmp_reply(void)
{
	struct iphdr* iph = (struct iphdr*) buf;
	struct icmphdr* pingh = (struct icmphdr*) (sizeof(struct iphdr) + buf);
	struct kvec vec = {
		.iov_base = buf,
		.iov_len = PACKET_BUF_LEN
	};
	struct msghdr msg;

	int ret = kernel_recvmsg(sock, &msg, &vec, 1, PACKET_BUF_LEN, 0);
	if (ret <= 0) {
		printk(KERN_ALERT "Error receiving IP packet.\n");
		return -1;
	} else if (iph->saddr == sa.sin_addr.s_addr && iph->protocol == IPPROTO_ICMP && pingh->type == ICMP_ECHOREPLY && ntohs(pingh->un.echo.id) == PING_ID)
		printk(KERN_INFO "%d bytes ICMP echo reply from %pI4, seq = %hu, ttl = %hhu\n", ret, &iph->saddr, ntohs(pingh->un.echo.sequence), iph->ttl);
	else
		printk(KERN_INFO "Unknown packet from %pI4\n", &iph->saddr);

	return 0;
}

/*
 * sysfs_write - запись данных из userspace в sysfs
 * Строка из sysfs файла интерпретируется как IP адрес,
 * на который выполняется отправка ICMP эхо запрос
 */
static ssize_t sysfs_write(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count)
{
	printk(KERN_INFO "Importing IP address via sysfs\n");
	u8* ip = (u8*) &sa.sin_addr.s_addr;
	if (sscanf(buf, "%hhu.%hhu.%hhu.%hhu", ip, &ip[1], &ip[2], &ip[3]) < 4)
		printk(KERN_ALERT "Not recognized input. Expecting IP address.\n");
	else /*if (kernel_connect(sock, (struct sockaddr*) &sa, sizeof(sa), 0) < 0) {
		sock_release(sock);
		printk(KERN_ALERT "Error connecting socket.\n");
	} else*/ {
		int ret = send_icmp_request();
		if (ret < 0)
			printk(KERN_ALERT "Error sending IP packet.\n");
		else {
			printk(KERN_INFO "Sent %d bytes of ICMP echo request to %pI4\n", ret, &sa.sin_addr.s_addr);
			recv_icmp_reply();
		}
	}
	return count;
}


static struct kobj_attribute sysfs_attr = __ATTR(KOBJ_FILE_NAME, 0660, sysfs_read, sysfs_write);

/*
 * setup_module - инициализация модуля
 * Создаётся sysfs объект и INET сокет
 */
static int __init setup_module(void)
{
	kobjp = kobject_create_and_add(KOBJ_DIR_NAME, kernel_kobj);
	if (! kobjp)
		return -ENOMEM;

	if (sysfs_create_file(kobjp, &sysfs_attr.attr)) {
		printk(KERN_INFO "Can't create sysfs file\n");
		kobject_put(kobjp);
	} else
		printk(KERN_INFO "Place IP address to /sys/kernel/%s/%s\n", KOBJ_DIR_NAME, __stringify(KOBJ_FILE_NAME));

    if (sock_create(AF_INET, SOCK_RAW, IPPROTO_ICMP, &sock) < 0) {
        printk("Socket creation failure.\n");
        return -1;
    }

	return 0;
}

/*
 * shutdown_module - высвобождение ресурсов перед выгрузкой модуля
 */
static void __exit shutdown_module(void)
{
	kobject_put(kobjp);
	printk(KERN_INFO "Sysfs object deleted\n");
	sock_release(sock);
}


module_init(setup_module);
module_exit(shutdown_module);
