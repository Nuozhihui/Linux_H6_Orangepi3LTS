/*
 * @Copyright (C), 2020 : YBS
 * @内容简述: 
 * @version: 
 * @Author: embeded
 * @LastEditors: embeded
 * @LastEditTime: 2021-10-09 11:44:46
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
 
int main(int argc, char **argv)
{
        int fd;
        int val=1;
        char buffer[80];
        fd = open("/dev/embeded_platform", O_RDWR);        //打开设备
        if(fd < 0){
            printf("can`t open!\n");
            return;
        }
        write(fd, &val, 4);
        for(int index = 0;index <5;index ++){
            printf("app run time %d\n",index);
            sleep(1);
        }
        read(fd,buffer,sizeof(buffer));   //读取globalmem设备中存储的数据
        return 0;
}


