
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/delay.h>
#include <linux/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/of_platform.h>
#include <linux/of_address.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/device.h>
#include <linux/timer.h>
#include <asm/uaccess.h>
#include <linux/sched.h>//jiffies在此头文件中定义
#include <linux/fcntl.h>

#define DEV_NAME     "embeded_platform"
#define DEV_COUNT    1

/*设备结构体*/
struct embededplatform_dev{
    dev_t devid;                /* 设备号 */
    struct cdev cdev;           /* 字符设备 */
    struct class *class;        /* 类结构体 */
    struct device *device;      /* 设备 */
    struct device_node *nd;     /* 设备节点 */
    struct timer_list timer; /* 定义定时器 */
    atomic_t keyvalue;
    struct fasync_struct *async_queue; /* 异步相关结构体 */
};

unsigned long flags;

//设备结构体实例
struct embededplatform_dev embededplatform_dev_er;

/* 定时器回调函数 */
void timer_callbackfunction(unsigned long arg){
    struct embededplatform_dev *dev = (struct embededplatform_dev *)arg;
    printk("embeded_platform timer_callbackfunction\n");
    atomic_set(&embededplatform_dev_er.keyvalue, 1);
    if(atomic_read(&dev->keyvalue)) { /* 一次完整的按键过程 */
 		if(dev->async_queue)
 			kill_fasync(&dev->async_queue, SIGIO, POLL_IN);
 	}
    mod_timer(&embededplatform_dev_er.timer, jiffies + msecs_to_jiffies(2000));
}

static int embeded_platform_open (struct inode *node, struct file *filp){
    printk("embeded_platform_open\n");
    filp->private_data = &embededplatform_dev_er;
    init_timer(&embededplatform_dev_er.timer); /* 初始化定时器 */
    embededplatform_dev_er.timer.function = timer_callbackfunction; /* 设置定时处理函数 */
    embededplatform_dev_er.timer.expires= jiffies + msecs_to_jiffies(5000);/* 超时时间 2 秒 */
    embededplatform_dev_er.timer.data = (unsigned long)&embededplatform_dev_er; /* 将设备结构体作为参数 */
    add_timer(&embededplatform_dev_er.timer); /* 启动定时器 */
    return 0;
}

static ssize_t embeded_platform_write (struct file *filp, const char __user *buf, size_t count, loff_t *off){	
    printk("embeded_platform_write\n");
    return 0;
}

ssize_t embeded_platform_read(struct file *filp, char __user *ubuf, size_t count, loff_t *ppos){
    unsigned char keyvalue = 0;
    int ret = 0;
    struct embededplatform_dev *dev = (struct embededplatform_dev *)filp->private_data;
    printk("embeded_platform_read %ld\n",count);
    keyvalue = atomic_read(&dev->keyvalue);
    atomic_set(&embededplatform_dev_er.keyvalue, 0);
    ret = copy_to_user(ubuf,&keyvalue,sizeof(keyvalue));
    return ret;
}

static int embeded_platform_fasync(int fd, struct file *filp, int on){
 	struct embededplatform_dev *dev = (struct embededplatform_dev *)filp->private_data;
    printk("embeded_platform_fasync \n");
 	return fasync_helper(fd, filp, on, &dev->async_queue);
}

static int embeded_platform_release (struct inode *node, struct file *filp){
    printk("embeded_platform_release \n");
    embeded_platform_fasync(-1, filp, 0);
    del_timer(&embededplatform_dev_er.timer); /* 删除定时器 */
    return 0;
}

static struct file_operations embeded_file_ops = {
     .owner = THIS_MODULE,
     .open  = embeded_platform_open,
     .write = embeded_platform_write,
     .read  = embeded_platform_read,
     .fasync = embeded_platform_fasync,
     .release = embeded_platform_release,
};

static int embeded_platform_probe(struct platform_device *pdev)
{
    int ret = 0;
    atomic_set(&embededplatform_dev_er.keyvalue, 0);
    printk("embeded_platform Probe\n");

    /* 申请设备号 */
    ret = alloc_chrdev_region(&embededplatform_dev_er.devid, 0, DEV_COUNT, DEV_NAME);
    if (ret < 0) {
        printk("embededplatform_dev_er chrdev_region err!\r\n");
        goto fail_devid;
    }
    
    /* 注册字符设备 */
    cdev_init(&embededplatform_dev_er.cdev, &embeded_file_ops);
    ret = cdev_add(&embededplatform_dev_er.cdev, embededplatform_dev_er.devid, DEV_COUNT);
    if (ret < 0) {
        goto fail_cdev;
    }

    /* 自动创建设备节点 */
    embededplatform_dev_er.class = class_create(embeded_file_ops.owner, DEV_NAME);
    if (IS_ERR(embededplatform_dev_er.class)) {
        ret = PTR_ERR(embededplatform_dev_er.class);
        goto fail_class;
    }

    embededplatform_dev_er.device = device_create(embededplatform_dev_er.class, NULL, embededplatform_dev_er.devid, NULL, DEV_NAME);
    if (IS_ERR(embededplatform_dev_er.device)) {
        ret = PTR_ERR(embededplatform_dev_er.device);
        goto fail_device;
    }
   
    printk("embeded_platform create success\n");
    return 0;

fail_device:
    class_destroy(embededplatform_dev_er.class);
fail_class:
    cdev_del(&embededplatform_dev_er.cdev);
fail_cdev:
    unregister_chrdev_region(embededplatform_dev_er.devid, DEV_COUNT);
fail_devid:
    return ret;
}

static int embeded_platform_remove(struct platform_device *pdev)
{
    printk("embeded_platform_exit\r\n");
    /* 删除字符设备 */
    cdev_del(&embededplatform_dev_er.cdev);
    /* 释放字符设号 */
    unregister_chrdev_region(embededplatform_dev_er.devid, DEV_COUNT);
    /* 摧毁设备 */
    device_destroy(embededplatform_dev_er.class, embededplatform_dev_er.devid);
    /* 摧毁类 */
    class_destroy(embededplatform_dev_er.class);
    return 0;
}

static struct of_device_id embeded_match_table[] = {
    { .compatible = "embeded,platform-test"},
    {},
};

static struct platform_driver embeded_platform_driver = {
    .driver = {
        .name = "embeded_platform",
        .owner = THIS_MODULE,
        .of_match_table = embeded_match_table,
    },
    .remove     = embeded_platform_remove,
    .probe      = embeded_platform_probe,

};

static int embeded_platformdriver_init(void)
{
    return platform_driver_register(&embeded_platform_driver);
}


static void embeded_platformdriver_exit(void)
{
    platform_driver_unregister(&embeded_platform_driver);
}

module_init(embeded_platformdriver_init);
module_exit(embeded_platformdriver_exit);

MODULE_AUTHOR("embeded");
MODULE_DESCRIPTION("embeded platform driver");
MODULE_LICENSE("GPL");
