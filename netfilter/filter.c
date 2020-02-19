/*
 * filter.c: Фильтрация TCP пакетов по порту, заданному в sysfs файле
 *
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/netfilter.h>
//#include <linux/netfilter_ipv4.h>
#include <linux/string.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/tcp.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrey Tarasov");
MODULE_DESCRIPTION("Test module for netfilter");


// Имя директории в sysfs
#define KOBJ_DIR_NAME	"myfilter"

// Имя атрибута/файла в sysfs
#define KOBJ_FILE_NAME	port


//
static u16 port = 80;

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


unsigned int catch_packet(uint hooknum,
						struct sk_buff *skb,
						const struct net_device *in,
						const struct net_device *out,
						int (*okfn)(struct sk_buff *))
{
	return NF_DROP;
}

/* Структура для регистрации функции перехватчика входящих ip пакетов */
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
	nf_unregister_net_hook(&nfhops);
	printk(KERN_INFO "Netfilter hook deregistered\n");
}


module_init(setup_module);
module_exit(shutdown_module);
