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
#include <linux/interrupt.h>

#define CLASS_NAME     "embeded_key_class"
#define DEV_NAME       "embeded_key"
#define KEY_COUNT    1


/*设备结构体*/
struct embeded_dev{
    dev_t devid;                /* 设备号 */
    struct cdev cdev;           /* 字符设备 */
    struct class *class;        /* 类结构体 */
    struct device *device;      /* 设备 */
    struct device_node *nd;     /* 设备节点 */

    int irq_gpio;
    int irq_mode;
    int irq;
    unsigned int ev_press ;
    unsigned int key_val;//全局变量
};

//设备结构体实例
struct embeded_dev my_embeded_dev;

DECLARE_WAIT_QUEUE_HEAD(button_waitq);//注册一个等待队列button_waitq


ssize_t key_read(struct file *filp, char __user *ubuf, size_t count, loff_t *ppos){
    int ret = -1;

    printk("key_read\n");
    wait_event_interruptible(button_waitq, my_embeded_dev.ev_press);
    ret = copy_to_user(ubuf, &my_embeded_dev.key_val, 1);//将取得的按键值传给上层应用
    my_embeded_dev.ev_press = 0;//按键已经处理可以继续睡眠
    if (ret)
    {
        printk(KERN_ERR "copy_to_user fail\n");
        return -EINVAL;
    }
    return 0;
}

static struct file_operations key_fops = {
     .owner = THIS_MODULE,
     .read  = key_read,
};

static irqreturn_t embeded_key_irq(int irq, void *dev_id) //中断函数
{
    my_embeded_dev.key_val = gpio_get_value(my_embeded_dev.irq_gpio);
    my_embeded_dev.ev_press = 1;
    wake_up_interruptible(&button_waitq);
    printk("Enter embeded_key_irq !\n");
    return IRQ_HANDLED;
}

static int key_probe(struct platform_device *pdev)
{
    int ret = 0;
    enum of_gpio_flags flag;
    printk("embededkey Probe\n");

    /* 申请设备号 */
    ret = alloc_chrdev_region(&my_embeded_dev.devid, 0, KEY_COUNT, DEV_NAME);
    if (ret < 0) {
        printk("my_embeded_dev chrdev_region err!\r\n");
        goto fail_devid;
    }
    /* 注册字符设备 */
    my_embeded_dev.cdev.owner = key_fops.owner;
    cdev_init(&my_embeded_dev.cdev, &key_fops);
    ret = cdev_add(&my_embeded_dev.cdev, my_embeded_dev.devid, KEY_COUNT);
    if (ret < 0) {
        goto fail_cdev;
    }
    /* 自动创建设备节点 */
    my_embeded_dev.class = class_create(key_fops.owner, CLASS_NAME);
    if (IS_ERR(my_embeded_dev.class)) {
        ret = PTR_ERR(my_embeded_dev.class);
        goto fail_class;
    }
    my_embeded_dev.device = device_create(my_embeded_dev.class, NULL, my_embeded_dev.devid, NULL, DEV_NAME);
    if (IS_ERR(my_embeded_dev.device)) {
        ret = PTR_ERR(my_embeded_dev.device);
        goto fail_device;
    }

    /* 获取设备树的属性内容 */
    my_embeded_dev.nd = pdev->dev.of_node;
    if (my_embeded_dev.nd == NULL) {
        printk("can't find num");
        ret = -EINVAL;
        goto fail_findnd;
    }
    /* 获取GPIO */
    my_embeded_dev.irq_gpio = of_get_named_gpio_flags(my_embeded_dev.nd, "embeded_gpio", 0, &flag);
    if (my_embeded_dev.irq_gpio < 0) {
        printk("can't find key gpio");
        ret = -EINVAL;
        goto fail_findnd;
    } else {
        printk("key gpio num = %d\r\n", my_embeded_dev.irq_gpio);
    }

    my_embeded_dev.irq_mode = flag;
    my_embeded_dev.irq = gpio_to_irq(my_embeded_dev.irq_gpio);
    if (my_embeded_dev.irq) {
            /* 申请IO */
            ret = devm_gpio_request(&pdev->dev,my_embeded_dev.irq_gpio, "embeded_key");
            if (ret < 0) {
                printk("failde to request the key gpio\r\n");
                ret = -EINVAL;
                goto fail_findnd;
            }
    }else{
        printk("failde to gpio_to_irq\r\n");
    }

    ret = request_irq(my_embeded_dev.irq, embeded_key_irq, my_embeded_dev.irq_mode, "embeded_key", &my_embeded_dev);
    if (ret != 0)
    {
        free_irq(my_embeded_dev.irq, &my_embeded_dev);
        printk("failde to request irq\r\n");
        ret = -EINVAL;
        goto fail_findnd;
    }

    printk("embeded key create success\n");
    return 0;

fail_findnd:
    device_destroy(my_embeded_dev.class, my_embeded_dev.devid);
fail_device:
    class_destroy(my_embeded_dev.class);
fail_class:
    cdev_del(&my_embeded_dev.cdev);
fail_cdev:
    unregister_chrdev_region(my_embeded_dev.devid, KEY_COUNT);
fail_devid:
    return ret;
}

static int key_remove(struct platform_device *pdev)
{
    printk("embededkey_exit\r\n");
    /* 删除字符设备 */
    cdev_del(&my_embeded_dev.cdev);
    /* 释放字符设号 */
    unregister_chrdev_region(my_embeded_dev.devid, KEY_COUNT);
    /* 摧毁设备 */
    device_destroy(my_embeded_dev.class, my_embeded_dev.devid);
    /* 摧毁类 */
    class_destroy(my_embeded_dev.class);
    free_irq(my_embeded_dev.irq, &my_embeded_dev);
    return 0;
}

static struct of_device_id key_match_table[] = {
    { .compatible = "embeded,key",},
    {},
};

static struct platform_driver key_driver = {
    .driver = {
        .name = "embeded-key",
        .owner = THIS_MODULE,
        .of_match_table = key_match_table,
    },
    .probe = key_probe,
    .remove = key_remove,
};

static int embededkey_init(void)
{
    return platform_driver_register(&key_driver);
}
module_init(embededkey_init);

static void embededkey_exit(void)
{
    platform_driver_unregister(&key_driver);
}
module_exit(embededkey_exit);

MODULE_AUTHOR("embeded");
MODULE_DESCRIPTION("embeded key driver");
MODULE_ALIAS("platform:embeded-key");
MODULE_LICENSE("GPL");
