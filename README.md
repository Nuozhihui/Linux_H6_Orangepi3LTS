

# Linux_H6_Orangepi3LTS

 Linux_H6_Orangepi3LTS



# 目录

* Linux_.H6_Orangepi3LTS
  * Headers
    * hello
    * linux-headers-next-sun50iw6_3.0.4_arm64.deb





# 常用命令

``` bash
sudo poweroff 	#关机
sudo reboot 	#重启
```



# Headers 

## Headers 的安装

### 使用Orangepi 编译

直接安装

```c
dpkg -i linux-headers-next-sun50iw6_3.0.4_arm64.deb
```

## 使用X86 编译

设置头文件目录

## 运行hello程序

编译

```bash
make
```

#### 安装卸载驱动和

```bash
insmod platform_driver.ko
rmmod platform_driver
```

#### 内核驱动信息打印

```bash
dmesg | grep "Hello"
dmesg | grep "platform_driver"
dmesg | grep "embeded_platform"
dmesg -c
```

# Git

```bash
git config --global user.name "Nuozhihui"
git config --global user.email 1285574579@qq.com
```

# linux_driver目录

hello world



# 修改DTS，Linux内核文件

路径

```bash
cd /home/jinchang/桌面/oPI/SDK/orangepi-build/kernel/orange-pi-5.16-sunxi64/arch/arm64/boot/dts
```

![image-20230112003424995](C:\Users\JINCHANG\AppData\Roaming\Typora\typora-user-images\image-20230112003424995.png)

插入DTS

注意位置！！

![image-20230112035621576](C:\Users\JINCHANG\AppData\Roaming\Typora\typora-user-images\image-20230112035621576.png)

# 一、查看 deb 信息的命令：

```bash
查看deb 信息命令内核
dpkg --get-selections| grep linux
```



![image-20230112060355289](C:\Users\JINCHANG\AppData\Roaming\Typora\typora-user-images\image-20230112060355289.png)

``` bash
卸载DTS的包（对应）----需要更改什么就卸载什么
sudo apt-get remove --purge linux-dtb-next-sun50iw6

安装新的DTS的包（对应）
sudo dpkg -i linux-dtb-next-sun50iw6_3.0.8_arm64.deb

```

查看是否挂载

```bash
ls proc/device-tree/ | grep test_demo
```

# 内核污染

## 检查污染

加载专有的或非 GPL 兼容的模块或未签名的模块将在运行的内核中设置一个“taint”标志。在内核日志中检查内核污染状态：

```bash
journalctl -k | grep taint
```

## 项目污染

```bash
root@orangepi3-lts:/home/orangepi/Desktop/Date/Linux_H6_Orangepi3LTS/Linux_Driver/linux_driver/2# dmesg | grep "platform_driver"
[  301.020876] platform_driver: loading out-of-tree module taints kernel.
[  301.020961] platform_driver: module verification failed: signature and/or required key missing - tainting kernel
```

> 内核在注意到内部问题（“kernel bug”）、可恢复错误（“kernel oops”）或不可恢复错误（“kernel [panic](https://so.csdn.net/so/search?q=panic&spm=1001.2101.3001.7020)”）时会打印 受污染状态



# 处理并发和竞争的机制

原⼦操作、⾃旋锁、信号量、互斥体

## 原⼦操作

Linux内核提供了⼀组原⼦操作API函数来完成此功能，Linux内核提供了两组原⼦操作API函数，⼀组是对整形 变量进⾏操作的，⼀组是对位进⾏操作的。

## ⾃旋锁

⾃旋锁短时期的轻量级加锁。

⻓时间持有锁信号量。

### 普通⾃旋锁

![image-20230115234236775](C:\Users\JINCHANG\AppData\Roaming\Typora\typora-user-images\image-20230115234236775.png)

### 中断⾃旋锁

![image-20230115234252257](C:\Users\JINCHANG\AppData\Roaming\Typora\typora-user-images\image-20230115234252257.png)





## 字符驱动设备



# GPIO

## Sys的方式控制GPIO

参考:https://www.jianshu.com/p/8e2449475c11



查看GPIO

```b
cat /sys/kernel/debug/gpio
```

在/sys/class/gpio暴露GPIO

```b
cd /sys/class/gpio
echo 354 > export
```



计算GPIO数字（PL2=354）

（字母序列号-1）*32 +尾部数字

PL2的gpio号就是354（计算方法（12-1）*32 + 2） ## 字母L是字母表第12个字母



进入到GPIO354目录内

配置输出模式

```B
echo out > direction
```



配置输出电平

```b
echo 0 > value
echo 1 > value
```

## 驱动开发的方式控制GPIO

DTS

![image-20230123222732804](C:\Users\JINCHANG\AppData\Roaming\Typora\typora-user-images\image-20230123222732804.png)

PIO:A~H

R_PIO:L~M

![image-20230123222757038](C:\Users\JINCHANG\AppData\Roaming\Typora\typora-user-images\image-20230123222757038.png)

## 使用DTS设备树

参考:https://blog.csdn.net/qq_63231786/article/details/124386244

# 正点原子历程

## 1.历程

> /***************************************************************
>
> 文件名   : chrdevbaseApp.c
>
> 作者    : 左忠凯
>
> 版本    : V1.0
>
> 描述    : chrdevbase驱测试APP。
>
> 其他    : 使用方法：./chrdevbase /dev/chrdevbase <1>|<2>
>
> ​       argv[2] 1:读文件
>
> ​       argv[2] 2:写文件    
>
> ***************************************************************/

### 使用

```bash
./chrdevbase /dev/chrdevbase 1
./chrdevbase /dev/chrdevbase 2
```





2.历程
