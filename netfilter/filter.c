/*
 * filter.c: Фильтрация TCP пакетов по порту, заданному в sysfs файле
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netfilter_ipv4.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/ip.h>
#include <linux/tcp.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrey Tarasov");
MODULE_DESCRIPTION("Test module for netfilter");


// Имя директории в sysfs
#define KOBJ_DIR_NAME	"myfilter"

// Имя атрибута/файла в sysfs
#define KOBJ_FILE_NAME	port


// Номер TCP порта, который следует фильтровать
static u16 port = 23;

static struct kobject *kobjp;

/*
 * sysfs_read - вывод данных в userspace через sysfs
 */
static ssize_t sysfs_read(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	printk(KERN_INFO "Reading sysfs\n");
	return sprintf(buf, "TCP port number to filter: %d\n", port);
}

/*
 * sysfs_write - ввод данных из userspace через sysfs
 */
static ssize_t sysfs_write(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count)
{
	printk(KERN_INFO "Writing to sysfs\n");
	if (! sscanf(buf, "%hu", &port))
		printk(KERN_ALERT "Not recognized input. Expecting integer number.\n");
	return count;
}


static struct kobj_attribute sysfs_attr = __ATTR(KOBJ_FILE_NAME, 0660, sysfs_read, sysfs_write);


/*
 * catch_packet - Перехватчик IP пакетов
 */
unsigned int catch_packet(void *priv, struct sk_buff *skb, const struct nf_hook_state *state)
{
	struct iphdr *ip_header = (struct iphdr *) skb_network_header(skb);
	if (ip_header->protocol == IPPROTO_TCP) {
		struct tcphdr* tcp_header = (struct tcphdr *) skb_transport_header(skb);
//		printk(KERN_INFO "Source Port: %u\n", tcp_header->source);
		if (tcp_header->dest == htons(port)) {
			printk(KERN_INFO "Droped packet to: %pI4:%u\n", &ip_header->daddr, port);
			return NF_DROP;
		}
	}
	return NF_ACCEPT;
}


// Структура для регистрации функции перехватчика входящих ip пакетов
static struct nf_hook_ops nfhops = {
	// Заполняем структуру для регистрации hook функции
	// Указываем имя функции, которая будет обрабатывать пакеты
	.hook = catch_packet,
	// Указываем семейство протоколов
	.pf = PF_INET,
	// Указываем, в каком месте будет срабатывать функция
	.hooknum = NF_INET_LOCAL_OUT,
	// Выставляем самый высокий приоритет для функции
	.priority = NF_IP_PRI_FIRST
};


/*
 * setup_module - инициализация модуля
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
		printk(KERN_INFO "Created sysfs entry /sys/kernel/%s/%s\n", KOBJ_DIR_NAME, __stringify(KOBJ_FILE_NAME));

	// Регистрируем netfilter hook
	nf_register_net_hook(&init_net, &nfhops);

	return 0;
}

/*
 * shutdown_module - закрытие модуля
 */
static void __exit shutdown_module(void)
{
	kobject_put(kobjp);
	printk(KERN_INFO "Sysfs object deleted\n");
	nf_unregister_net_hook(&init_net, &nfhops);
	printk(KERN_INFO "Netfilter hook deregistered\n");
}


module_init(setup_module);
module_exit(shutdown_module);
