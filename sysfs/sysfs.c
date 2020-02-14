/*
 * sysfs.c: взаимодействие между ядром и пространством пользователя
 * через виртуальную файловую систему sysfs
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include<linux/sysfs.h>
#include<linux/kobject.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrey Tarasov");
MODULE_DESCRIPTION("Test module for sysfs");

// Размер буфера
#define BUF_SIZE	1024

// Имя директории в sysfs
#define KOBJ_DIR_NAME	"test"

// Имя атрибута/файла в sysfs
#define KOBJ_FILE_NAME	message


// Буфер для ввода/вывода данных через sysfs
static char kbuffer[BUF_SIZE] = "Hello from sysfs!\n";

static struct kobject *kobjp;

/*
 * sysfs_read - вывод данных из буфера в userspace через sysfs
 */
static ssize_t sysfs_read(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	printk(KERN_INFO "Reading sysfs\n");
	return sprintf(buf, "%s", kbuffer);
}

/*
 * sysfs_write - ввод данных в буфер из userspace через sysfs
 */
static ssize_t sysfs_write(struct kobject *kobj, struct kobj_attribute *attr,const char *buf, size_t count)
{
	printk(KERN_INFO "Writing to sysfs\n");
	strncpy(kbuffer, buf, count);
	kbuffer[count] = 0;
	return count;
}


static struct kobj_attribute sysfs_attr = __ATTR(KOBJ_FILE_NAME, 0660, sysfs_read, sysfs_write);

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

	return 0;
}

/*
 * shutdown_module - закрытие модуля
 */
static void __exit shutdown_module(void)
{
	kobject_put(kobjp);
	printk(KERN_INFO "Sysfs object deleted\n");
}


module_init(setup_module);
module_exit(shutdown_module);

