/*
 * devfile.c: взаимодействие между ядром и пространством пользователя
 * через файл символьного устройства
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/time.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <asm/uaccess.h>
#include "devfile.h"


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrey Tarasov");
MODULE_DESCRIPTION("My character device driver");


static int major_num;
static int busy = 0;
static char message[BUF_SIZE] = "First time opening device\n";
static char* mes_ptr;

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = mydev_read,
	.write = mydev_write,
	.open = mydev_open,
	.release = mydev_release
};


static int __init setup_device(void)
{
	major_num = register_chrdev(0, DEVICE_NAME, &fops);
	if (major_num < 0) {
		printk(KERN_ALERT "Could not regigister character  device.\nError code: %d\n", major_num);
		return(major_num);
	}

	printk(KERN_INFO "Device registered with major number %d\n", major_num);
	return 0;
}


static void __exit shutdown_device(void)
{
	unregister_chrdev(major_num, DEVICE_NAME);
	printk(KERN_INFO "Device unregistered\n");
}


module_init(setup_device);
module_exit(shutdown_device);


static int mydev_open(struct inode *inode, struct file *file)
{
	static time_t last = 0;
	struct timespec now;

	if (busy)
		return -EBUSY;
	++busy;

	getnstimeofday(&now);
	if (last)
		sprintf(message, "Last open was %ld seconds ago\n", now.tv_sec - last);
	mes_ptr = message;

	last = now.tv_sec;
	return 0;
}

/*
 *
 */
static int mydev_release(struct inode *inode, struct file *file)
{
	--busy;
	return 0;
}

/*
 *
 *
 */
static ssize_t mydev_read(struct file *filp,	/* see include/linux/fs.h   */
			   char *buffer,	/* buffer to fill with data */
			   size_t length,	/* length of the buffer     */
			   loff_t * offset)
{
	int counter = 0;

	if (! *mes_ptr)
		return 0;

	while (*mes_ptr && length) {
		put_user(*mes_ptr++, buffer++);
		--length;
		++counter;
	}

	return counter;
}

/*
 *
 */
static ssize_t mydev_write(struct file *filp, const char *buff, size_t len, loff_t * off)
{
	printk(KERN_ALERT "Write attempt to mydev.\n");
	return -EINVAL;
}
