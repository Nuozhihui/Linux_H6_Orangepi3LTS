/*
 * @Copyright (C), 2020 : YBS
 * @内容简述: 
 * @version: 
 * @Author: embeded
 * @LastEditors: embeded
 * @LastEditTime: 2021-10-13 11:52:09
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
        int temp;
        char buffer[3];
 
        fd = open("/dev/gy906", O_RDWR);   
        if(fd < 0)
            printf("can`t open!\n");
        read(fd,buffer,sizeof(buffer));  
        temp = (buffer[1] * 256 + buffer[0])*2 - 27315;
        printf("temp = %d.%d,read %d %d %d\n",temp/100,temp%100,buffer[0],buffer[1],buffer[2]);  //输出结果显示 
        return 0;
}


