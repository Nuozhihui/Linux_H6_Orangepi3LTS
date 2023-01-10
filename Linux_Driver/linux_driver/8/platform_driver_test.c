/*
 * @Copyright (C), 2020 : YBS
 * @内容简述: 
 * @version: 
 * @Author: embeded
 * @LastEditors: embeded
 * @LastEditTime: 2021-10-09 15:29:04
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include "poll.h"
 
int main(int argc, char **argv)
{
        int fd;
        int ret = 0;
        struct pollfd fds;
        unsigned char data;
        int val=1;
        char buffer[80];
        fd = open("/dev/embeded_platform", O_RDWR | O_NONBLOCK);        //打开设备
        if(fd < 0){
            printf("can`t open!\n");
            return;
        }
        printf("start poll!\n");
        /* 构造结构体 */
		fds.fd = fd;
		fds.events = POLLIN;

 		while (1) {
			printf("poll run\n");
			ret = poll(&fds, 1, 100);
			if (ret) { /* 数据有效 */
                printf("poll get data\n");
				ret = read(fd, &data, sizeof(data));
 				if(ret < 0) {
					/* 读取错误 */
 				} else {
 					if(data)
 						printf("key value = %d \r\n", data);
 				}
 			} else if (ret == 0) { /* 超时 */
 				/* 用户自定义超时处理 */
 			} else if (ret < 0) { /* 错误 */
 			/* 用户自定义错误处理 */
 			}
			usleep(200);
 		}
        return 0;
}


