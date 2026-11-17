/*
 * chardev.c: Dispositivo de caracteres con soporte de lectura y escritura
 * Basado en el Ejemplo 4-1 de LKMPG
 */
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <asm/uaccess.h>    /* para get_user y put_user */

/* Prototipos */
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

#define SUCCESS 0
#define DEVICE_NAME "chardev"
#define BUF_LEN 80

static int Major;
static int Device_Open = 0;
static char msg[BUF_LEN] = "Mensaje inicial por defecto.\n"; /* Buffer compartido */
static char *msg_Ptr;

static struct file_operations fops = {
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

int init_module(void)
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

void cleanup_module(void)
{
    unregister_chrdev(Major, DEVICE_NAME);
}

static int device_open(struct inode *inode, struct file *file)
{
    if (Device_Open)
        return -EBUSY;

    Device_Open++;
    /* Apuntamos al inicio del mensaje actual sin sobrescribirlo */
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

/* Módulo de LECTURA: envía datos del kernel al usuario */
static ssize_t device_read(struct file *filp, char *buffer, size_t length, loff_t *offset)
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

/* Módulo de ESCRITURA (Modificado): recibe datos del usuario hacia el kernel */
static ssize_t device_write(struct file *filp, const char *buff, size_t len, loff_t *off)
{
    int i;

    /* Copiamos byte a byte desde el espacio de usuario evitando desbordar BUF_LEN */
    for (i = 0; i < len && i < BUF_LEN - 1; i++) {
        get_user(msg[i], buff + i);
    }

    /* Terminamos la cadena con carácter nulo y reiniciamos el puntero */
    msg[i] = '\0';
    msg_Ptr = msg;

    printk(KERN_INFO "chardev: se escribieron %d bytes\n", i);
    return i; /* Retornamos la cantidad de bytes escritos */
}

MODULE_LICENSE("GPL");
