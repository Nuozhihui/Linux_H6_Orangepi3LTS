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

#define CLASS_NAME     "embeded_led_class"
#define DEV_NAME       "embeded_led"
#define POWERLED_COUNT    1


/*设备结构体*/
struct embeded_dev{
    dev_t devid;                /* 设备号 */
    struct cdev cdev;           /* 字符设备 */
    struct class *class;        /* 类结构体 */
    struct device *device;      /* 设备 */
    struct device_node *nd;     /* 设备节点 */
    int gpio_number;            /* gpio的编号 */
    char kbuf[100]; 
};

//设备结构体实例
struct embeded_dev my_embeded_dev;

static int led_open (struct inode *node, struct file *filp){
    printk("led_open\n");
    filp->private_data = &my_embeded_dev;
    return 0;
}

static ssize_t led_write (struct file *filp, const char __user *buf, size_t count, loff_t *off){
    int ret = 0;
    struct embeded_dev *dev = filp->private_data;
    printk("led_write\n");
    ret = copy_from_user(my_embeded_dev.kbuf, buf, count);
    if (ret < 0) {
        printk(KERN_ERR "copy_from_user fail\n");
        return -EINVAL;
    }

    if (my_embeded_dev.kbuf[0] == 0) {
        gpio_set_value(dev->gpio_number, 0);
    } else if (my_embeded_dev.kbuf[0] == 1) {
        gpio_set_value(dev->gpio_number, 1);
    }

    return 0;
}

ssize_t led_read(struct file *filp, char __user *ubuf, size_t count, loff_t *ppos){
    int ret = -1;
    struct embeded_dev *dev = filp->private_data;
    printk("led_read %ld\n",count);
    my_embeded_dev.kbuf[0] = gpio_get_value(dev->gpio_number);
    ret = copy_to_user(ubuf, my_embeded_dev.kbuf, count);
    if (ret)
    {
        printk(KERN_ERR "copy_to_user fail\n");
        return -EINVAL;
    }
    return 0;
}

static int led_release (struct inode *node, struct file *filp){
    printk("led_release\n");
    return 0;
}

static struct file_operations led_power = {
     .owner = THIS_MODULE,
     .open  = led_open,
     .write = led_write,
     .read  = led_read,
     .release = led_release,
};


static ssize_t led_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) 
{
        unsigned long on = simple_strtoul(buf, NULL, 10);
        printk("led_store\n");
        if(!strcmp(attr->attr.name, "embeded_led")){
                if(on){
                     gpio_set_value(my_embeded_dev.gpio_number, 1);
                }
                else{
                     gpio_set_value(my_embeded_dev.gpio_number, 0);
                }
        }
        return count;
}

static ssize_t led_show(struct device *dev,struct device_attribute *attr,char *buf)
{
        int tmp = 0;
	printk("led_show\n");
        if(!strcmp(attr->attr.name, "embeded_led")){
                tmp = gpio_get_value(my_embeded_dev.gpio_number);
                printk("embeded_led is %d \n",tmp);
                if(tmp>0)
                    return strlcpy(buf, "1\n", 3);
                else
                    return strlcpy(buf, "0\n", 3);
        }
        return 0;
}

static DEVICE_ATTR(embeded_led, S_IRWXU | S_IRWXG, led_show,led_store);

static int led_probe(struct platform_device *pdev)
{
    int ret = 0;
    enum of_gpio_flags flag;
    printk("led Probe\n");

    /* 申请设备号 */
    ret = alloc_chrdev_region(&my_embeded_dev.devid, 0, POWERLED_COUNT, DEV_NAME);
    if (ret < 0) {
        printk("my_embeded_dev chrdev_region err!\r\n");
        goto fail_devid;
    }
    /* 注册字符设备 */
    my_embeded_dev.cdev.owner = led_power.owner;
    cdev_init(&my_embeded_dev.cdev, &led_power);
    ret = cdev_add(&my_embeded_dev.cdev, my_embeded_dev.devid, POWERLED_COUNT);
    if (ret < 0) {
        goto fail_cdev;
    }
    /* 自动创建设备节点 */
    my_embeded_dev.class = class_create(led_power.owner, CLASS_NAME);
    if (IS_ERR(my_embeded_dev.class)) {
        ret = PTR_ERR(my_embeded_dev.class);
        goto fail_class;
    }
    my_embeded_dev.device = device_create(my_embeded_dev.class, NULL, my_embeded_dev.devid, NULL, DEV_NAME);
    if (IS_ERR(my_embeded_dev.device)) {
        ret = PTR_ERR(my_embeded_dev.device);
        goto fail_device;
    }
    if(sysfs_create_file(&(my_embeded_dev.device->kobj), &dev_attr_embeded_led.attr)){
        goto fail_findnd;
    }

    /* 获取设备树的属性内容 */
    my_embeded_dev.nd = pdev->dev.of_node;
    if (my_embeded_dev.nd == NULL) {
        printk("can't find num");
        ret = -EINVAL;
        goto fail_findnd;
    }
    /* 获取GPIO */
    my_embeded_dev.gpio_number = of_get_named_gpio_flags(my_embeded_dev.nd, "embeded_gpio", 0, &flag);
    if (my_embeded_dev.gpio_number < 0) {
        printk("can't find led gpio");
        ret = -EINVAL;
        goto fail_findnd;
    } else {
        printk("led gpio num = %d\r\n", my_embeded_dev.gpio_number);
    }
    /* 申请IO */
    ret = devm_gpio_request(&pdev->dev,my_embeded_dev.gpio_number, "embeded_led");
    if (ret < 0) {
        printk("failde to request the led gpio\r\n");
        ret = -EINVAL;
        goto fail_findnd;
    }
    /* 设置IO为模式 */
    ret = gpio_direction_output(my_embeded_dev.gpio_number, (flag == OF_GPIO_ACTIVE_LOW) ? 0:1);
    if (ret < 0) {
        ret = -EINVAL;
        goto fail_findnd;
    }

    printk("kemp led create success\n");
    return 0;

fail_findnd:
    device_destroy(my_embeded_dev.class, my_embeded_dev.devid);
fail_device:
    class_destroy(my_embeded_dev.class);
fail_class:
    cdev_del(&my_embeded_dev.cdev);
fail_cdev:
    unregister_chrdev_region(my_embeded_dev.devid, POWERLED_COUNT);
fail_devid:
    return ret;
}

static int led_remove(struct platform_device *pdev)
{
    printk("dtsled_exit\r\n");
    /* 删除字符设备 */
    cdev_del(&my_embeded_dev.cdev);
    /* 释放字符设号 */
    unregister_chrdev_region(my_embeded_dev.devid, POWERLED_COUNT);
    /* 摧毁设备 */
    device_destroy(my_embeded_dev.class, my_embeded_dev.devid);
    /* 摧毁类 */
    class_destroy(my_embeded_dev.class);
    return 0;
}

static struct of_device_id led_match_table[] = {
    { .compatible = "embeded,led",},
    {},
};

static struct platform_driver led_driver = {
    .driver = {
        .name = "embeded-led",
        .owner = THIS_MODULE,
        .of_match_table = led_match_table,
    },
    .probe = led_probe,
    .remove = led_remove,
};

static int led_init(void)
{
    return platform_driver_register(&led_driver);
}
module_init(led_init);

static void led_exit(void)
{
    platform_driver_unregister(&led_driver);
}
module_exit(led_exit);

MODULE_AUTHOR("embeded");
MODULE_DESCRIPTION("embeded led driver");
MODULE_ALIAS("platform:embeded-led");
MODULE_LICENSE("GPL");
