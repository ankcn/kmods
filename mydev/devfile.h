
#define DEVICE_NAME     "mydev"
#define BUF_SIZE	512


static int mydev_open(struct inode *, struct file *);
static int mydev_release(struct inode *, struct file *);
static ssize_t mydev_read(struct file *, char *, size_t, loff_t *);
static ssize_t mydev_write(struct file *, const char *, size_t, loff_t *);

