#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/err.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/of_device.h>

#define I2C_NAME "gy906"

static int major;
static struct i2c_client *iic_client=NULL;
static dev_t devid;                /* 设备号 */
static struct cdev cdev;           /* 字符设备 */
static struct class *class;        /* 类结构体 */
static struct device *device;      /* 设备 */

static int gy906_open(struct inode *inode,struct file *fd)
{
    fd->private_data=(void*)iic_client;
    return 0;
}


static ssize_t gy906_read(struct file *filp, char __user *buf, size_t size, loff_t *offset) 
{ 
    int ret; 
    //int temp;
    int timecount = 0;
    struct i2c_msg msg[2]; 
    unsigned char read_buf[3];
    unsigned char write_buf[1] = {0x07};
    printk("gy906 i2c read %d\n",iic_client->addr);
    
    msg[0].addr  = iic_client->addr;                //distination基址 
    msg[0].len   = 1;                               //length 地址长度=2byte 
    msg[0].buf   = &write_buf[0];                   //source 连续两个地址长度
    msg[0].flags = 0;                               //write flag 
    /* 再把数据读出 */ 
    msg[1].addr  = iic_client->addr;                //source 
    msg[1].len   = 3;                               //length 数据长度=1byte 
    msg[1].buf   = &read_buf[0];                       //distination 
    msg[1].flags = I2C_M_RD;                        //read flag 

    while(1){
        timecount ++;
        if(timecount >= 50)
            break;
        ret = i2c_transfer(iic_client->adapter, msg, 2); 
        if(ret==2){ 
            //printk("gy906 : i2c_transfer read success %d %d %d!\n",read_buf[0],read_buf[1],read_buf[2]); 
            //temp = (read_buf[1] * 256 + read_buf[0])*2 - 27315;
            //printk("temp is %d\n",temp);
            break;
        }
    }
    ret = copy_to_user(buf, read_buf, size);
    if (ret)
    {
        printk(KERN_ERR "copy_to_user fail\n");
        return -EINVAL;
    }
    return 0;
}



static struct file_operations iic_fops = {
    .owner = THIS_MODULE,
    .read = gy906_read,
    .open = gy906_open,
};


static struct of_device_id gy906_ids[] = {
    {.compatible = "gy906"},
    {}   
};
static const struct i2c_device_id gy906_id[] = { 
    {I2C_NAME, 0}, 
    {}   
};
MODULE_DEVICE_TABLE(i2c, gy906_id);
static int gy906_probe(struct i2c_client *client,const struct i2c_device_id *id)
{
    int ret = 0;
    printk(KERN_INFO "%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    ret = alloc_chrdev_region(&devid, 0, 1, I2C_NAME);
    major = MAJOR(devid);
    if (ret < 0) {
        printk("gy906 alloc_chrdev_region err!\r\n");
        goto fail_devid;
    }
    cdev_init(&cdev, &iic_fops);
    ret = cdev_add(&cdev, devid, 1);
    if (ret < 0) {
        goto fail_cdev;
    }

    class = class_create(THIS_MODULE, I2C_NAME);
    if (IS_ERR(class)) {
        ret = PTR_ERR(class);
        goto fail_class;
    }
    device  = device_create(class,&client->dev, MKDEV(major, 0), NULL, I2C_NAME); 
    if (IS_ERR(device)) {
        ret = PTR_ERR(device);
        goto fail_device;
    }
    iic_client = client;
    return 0;


fail_device:
    class_destroy(class);
fail_class:
    cdev_del(&cdev);
fail_cdev:
    unregister_chrdev_region(devid, 1);
fail_devid:
    return ret;
}
static int gy906_remove(struct i2c_client *client)
{
    printk(KERN_INFO "%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    /* 删除字符设备 */
    cdev_del(&cdev);
    /* 释放字符设号 */
    unregister_chrdev_region(devid, 1);
    /* 摧毁设备 */
    device_destroy(class, MKDEV(major, 0));
    /* 摧毁类 */
    class_destroy(class);
    return 0;
}
static struct i2c_driver gy906_driver = { 
    .driver = { 
        .name = I2C_NAME, 
        .owner = THIS_MODULE, 
        .of_match_table = of_match_ptr(gy906_ids), 
        },   
    .probe      = gy906_probe, 
    .remove     = gy906_remove, 
    .id_table   = gy906_id,
};
static int __init gy906_init(void)
{
    printk(KERN_INFO "%s %s %d\n", __FILE__, __FUNCTION__, __LINE__);
    return i2c_add_driver(&gy906_driver);
}
module_init(gy906_init);

static void __exit gy906_exit(void)
{
    i2c_del_driver(&gy906_driver);
}
module_exit(gy906_exit);


MODULE_AUTHOR("embeded");
MODULE_DESCRIPTION("gy960 driver");
MODULE_LICENSE("GPL");



