/*
 * @Copyright (C), 2020 : YBS
 * @内容简述: 
 * @version: 
 * @Author: embeded
 * @LastEditors: embeded
 * @LastEditTime: 2021-10-09 15:47:30
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "signal.h"

int fd;

static void sigio_signal_func(int signum){
 		int err = 0;
 		unsigned int keyvalue = 0;
 		err = read(fd, &keyvalue, sizeof(keyvalue));
		if(err < 0) {
			/* 读取错误 */
            printf("sigio read error \n");
 		} else {
 			printf("sigio signal! key value=%d\r\n", keyvalue);
 		}
}

 
int main(int argc, char **argv)
{
        
        int val=1;
        int flags = 0;
        char buffer[80];
        fd = open("/dev/embeded_platform", O_RDWR);        //打开设备
        if(fd < 0){
            printf("can`t open!\n");
            return;
        }
        /* 设置信号 SIGIO 的处理函数 */
 		signal(SIGIO, sigio_signal_func);

 		fcntl(fd, F_SETOWN, getpid()); /* 将当前进程的进程号告诉给内核 */
 		flags = fcntl(fd, F_GETFD); /* 获取当前的进程状态 */
 		fcntl(fd, F_SETFL, flags | FASYNC);/* 设置进程启用异步通知功能 */

 		while(1) {
 			sleep(2);
 		}
        return 0;
}


