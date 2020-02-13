/**
 *  procfs.c -  create a "file" in /proc
 *
 */

#include <linux/module.h>	/* Specifically, a module */
#include <linux/kernel.h>	/* We're doing kernel work */
#include <linux/proc_fs.h>	/* Necessary because we use the proc fs */
#include <linux/string.h>


#define PROCFS_MAX_SIZE		1024
#define PROCFS_NAME 		"buffer1k"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrey Tarasov");


/**
 * This structure hold information about the /proc file
 *
 */
static struct proc_dir_entry *Our_Proc_File;

/**
 * The buffer used to store character for this module
 *
 */
static char procfs_buffer[PROCFS_MAX_SIZE] = "Test message from procfs\n";

/**
 * This function is called when the /proc file is read
 *
 */
static ssize_t procfile_read(struct file *file, char __user *ubuf, size_t count, loff_t *ppos)
{
	count = strlen(procfs_buffer);
	if (*ppos >= count - 1)
		return 0;
	printk(KERN_INFO "procfile_read (/proc/%s) called\n", PROCFS_NAME);
	return simple_read_from_buffer(ubuf, count, ppos, procfs_buffer, PROCFS_MAX_SIZE);
}


/**
 * This function is called with the /proc file is written
 *
 */
static ssize_t procfile_write(struct file *file, const char __user *ubuf, size_t count, loff_t *ppos)
{
	printk(KERN_INFO "procfile_write (/proc/%s) called\n", PROCFS_NAME);
	ssize_t ret = simple_write_to_buffer(procfs_buffer, PROCFS_MAX_SIZE, ppos, ubuf, count);
	if (ret > 0)
		procfs_buffer[ret] = 0;
	return ret;
}


struct file_operations fops = {
    .read = procfile_read,
    .write = procfile_write,
    .owner = THIS_MODULE
};


/**
 *This function is called when the module is loaded
 *
 */
int init_module()
{
	/* create the /proc file */
	Our_Proc_File = proc_create(PROCFS_NAME, 0644, NULL, &fops);

	if (Our_Proc_File == NULL) {
		proc_remove(Our_Proc_File);
		printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", PROCFS_NAME);
		return -ENOMEM;
	}

	printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);
	return 0;	/* everything is ok */
}

/**
 *This function is called when the module is unloaded
 *
 */
void cleanup_module()
{
	proc_remove(Our_Proc_File);
	printk(KERN_INFO "/proc/%s removed\n", PROCFS_NAME);
}

