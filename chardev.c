#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define SUCCESS 0
#define DEVICE_NAME "chardev"
#define BUF_LEN 80

static int Major;
static int Device_Open = 0;
static char msg[BUF_LEN] = "Mensaje inicial por defecto.\n";
static char *msg_Ptr;

static int device_open(struct inode *inode, struct file *file)
{
    if (Device_Open)
        return -EBUSY;

    Device_Open++;
    msg_Ptr = msg;
    try_module_get(THIS_MODULE);
    return SUCCESS;
}

static int device_release(struct inode *inode, struct file *file)
{
    Device_Open--;
    module_put(THIS_MODULE);
    return 0;
}

static ssize_t device_read(struct file *filp, char __user *buffer, size_t length, loff_t *offset)
{
    int bytes_read = 0;

    if (*msg_Ptr == 0)
        return 0;

    while (length && *msg_Ptr) {
        put_user(*(msg_Ptr++), buffer++);
        length--;
        bytes_read++;
    }

    return bytes_read;
}

static ssize_t device_write(struct file *filp, const char __user *buff, size_t len, loff_t *off)
{
    int i;

    for (i = 0; i < len && i < BUF_LEN - 1; i++) {
        get_user(msg[i], buff + i);
    }

    msg[i] = '\0';
    msg_Ptr = msg;

    printk(KERN_INFO "chardev: se escribieron %d bytes\n", i);
    return i;
}

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

static int __init chardev_init(void)
{
    Major = register_chrdev(0, DEVICE_NAME, &fops);

    if (Major < 0) {
        printk(KERN_ALERT "Error al registrar el dispositivo: %d\n", Major);
        return Major;
    }

    printk(KERN_INFO "Major number asignado: %d\n", Major);
    printk(KERN_INFO "Crea el archivo con: 'mknod /dev/%s c %d 0'\n", DEVICE_NAME, Major);
    return SUCCESS;
}

static void __exit chardev_exit(void)
{
    unregister_chrdev(Major, DEVICE_NAME);
}

module_init(chardev_init);
module_exit(chardev_exit);

MODULE_LICENSE("GPL");
