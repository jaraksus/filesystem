#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include <linux/string.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sudakov Dmitry");
MODULE_DESCRIPTION("Mini-filesystem Linux module.");
MODULE_VERSION("0.1");

#define DEVICE_NAME "lkm_minifs"
#define MAJOR_NUM 238

/* Prototypes for device functions */
static loff_t device_llseek(struct file *, loff_t, int);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static int device_open_count = 0;
static int fs_size = 0;
static char *fs_ptr = NULL;

/* This structure points to all of the device functions */
static struct file_operations file_ops = {
    .llseek = device_llseek,
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

static loff_t device_llseek(struct file *filp, loff_t off, int whence) {
    loff_t newpos;

    switch(whence) {
        case 0: /* SEEK_SET */
            newpos = off;
            break;
        case 1: /* SEEK_CUR */
            newpos = filp->f_pos + off;
            break;
        case 2: /* SEEK_END */
            newpos = fs_size + off;
            break;
        default: /* can't happen */
            return -EINVAL;
    }
    if (newpos<0) return -EINVAL;
    filp->f_pos = newpos;
    return newpos;
}

static ssize_t device_read(struct file *flip, char *buffer, size_t len, loff_t *offset) {
    if (*offset == fs_size) {
        return 0;
    }

    int bytes_read = 0;
    while (len && *offset < fs_size) {
        put_user(*(fs_ptr + *offset), buffer++);
        len--;
        (*offset)++;
        bytes_read++;
    }

    return bytes_read;
}

static ssize_t device_write(struct file *flip, const char *buffer, size_t len, loff_t *offset) {
    if (fs_size == 0) {
        fs_ptr = kmalloc(2, GFP_KERNEL);
        fs_size = 2;
        memset(fs_ptr, 0, fs_size);
    }

    while (fs_size - (*offset) < len) {
        char* old_ptr = fs_ptr;
        fs_ptr = kmalloc(fs_size * 2, GFP_KERNEL);
        memset(fs_ptr, 0, fs_size * 2);
        memcpy(fs_ptr, old_ptr, fs_size);
        kfree(old_ptr);
        fs_size *= 2;
    }

    int i = 0;
    for (i = 0; i < len; ++i) {
        get_user(fs_ptr[*offset + i], buffer + i);
    }
    return len;
}

static int device_open(struct inode *inode, struct file *file) {
    /* If device is open, return busy */
    if (device_open_count) {
        return -EBUSY;
    }
    device_open_count++;
    try_module_get(THIS_MODULE);
    return 0;
}

/* Called when a process closes our device */
static int device_release(struct inode *inode, struct file *file) {
    /* Decrement the open counter and usage count. Without this, the module would not unload. */
    device_open_count--;
    module_put(THIS_MODULE);
    return 0;
}

static int __init lkm_example_init(void) {
    /* Try to register character device */
    int ok = register_chrdev(MAJOR_NUM, DEVICE_NAME, &file_ops);
    if (ok != 0) {
        printk(KERN_ALERT "Could not register device: %d\n", MAJOR_NUM);
        return ok;
    } else {
        printk(KERN_INFO "lkm_minifs module loaded with device major number %d\n", MAJOR_NUM);
        return 0;
    }
}
static void __exit lkm_example_exit(void) {
    kfree(fs_ptr);
    unregister_chrdev(MAJOR_NUM, DEVICE_NAME);
    printk(KERN_INFO "Goodbye, World!\n");
}

/* Register module functions */
module_init(lkm_example_init);
module_exit(lkm_example_exit);