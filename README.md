

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
insmod hello.ko
rmmod hello
```

#### 内核驱动信息打印

```bash
dmesg | grep "Hello"
```

