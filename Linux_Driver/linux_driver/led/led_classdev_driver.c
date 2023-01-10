/*
 * Driver for pwm demo on embeded board.
 *
 * Copyright (C) 2016, Zhongshan T-chip Intelligent Technology Co.,ltd.
 * Copyright 2006  JC.Lin
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/io.h>
#include <linux/of.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/leds.h> 

/*设备结构体*/
struct embeded_dev{
    struct device_node *nd;     /* 设备节点 */
    int gpio_number;            /* gpio的编号 */
    struct led_classdev ledclass;
};

struct embeded_dev my_embeded_dev;


static void led_set(struct led_classdev *led_cdev,enum led_brightness value)  
{
    printk("led_set\n"); 
    if (value == LED_OFF){
        gpio_direction_output(my_embeded_dev.gpio_number, 0);
    }else{
        gpio_direction_output(my_embeded_dev.gpio_number, 1);
    }
}

static int led_probe(struct platform_device *pdev)
{
    int ret = -1;  
    enum of_gpio_flags flag;

    printk("led Probe\n");

    /* 获取设备树的属性内容 */
    my_embeded_dev.nd = pdev->dev.of_node;
    if (my_embeded_dev.nd == NULL) {
        printk("can't find num");
        return -EINVAL;
    }
    /* 获取GPIO */
    my_embeded_dev.gpio_number = of_get_named_gpio_flags(my_embeded_dev.nd, "embeded_gpio", 0, &flag);
    if (my_embeded_dev.gpio_number < 0) {
        printk("can't find led gpio");
        return -EINVAL;
    } else {
        printk("led gpio num = %d\r\n", my_embeded_dev.gpio_number);
    }
    /* 申请IO */
    ret = devm_gpio_request(&pdev->dev,my_embeded_dev.gpio_number, "embeded_led");
    if (ret < 0) {
        printk("failde to request the led gpio\r\n");
        return -EINVAL;
    }
    /* 设置IO为模式 */
    ret = gpio_direction_output(my_embeded_dev.gpio_number, (flag == OF_GPIO_ACTIVE_LOW) ? 0:1);
    if (ret < 0) {
        return -EINVAL;
    }
    
    my_embeded_dev.ledclass.name = "embeded_led"; 
    my_embeded_dev.ledclass.brightness = 255;   
    my_embeded_dev.ledclass.brightness_set = led_set;  
    ret = led_classdev_register(NULL, &my_embeded_dev.ledclass);  
    if (ret < 0) {  
        printk(KERN_ERR "led_classdev_register failed\n");  
        return ret;  
    }  
    printk("led create success\n");
    return 0;
}

static int led_remove(struct platform_device *pdev){
    led_classdev_unregister(&my_embeded_dev.ledclass);
    printk("led remove success\n");
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

MODULE_AUTHOR("embeded>");
MODULE_DESCRIPTION("embeded led driver");
MODULE_ALIAS("platform:embeded-led");
MODULE_LICENSE("GPL");
