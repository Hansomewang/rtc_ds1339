#include<sys/types.h>
#include <stdio.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/ioctl.h> 
#include <time.h>

#include "gpio.h"
 
rtc_time rtc_tm;

int main()
{
	int fd;
    int ret;

	fd = open("/dev/rtc_time", O_RDWR);//可读可写打开文件
    printf("fd = %d\n",fd);
    if(fd == -1 )
    {
        printf("open /dev/rtc_time filed!\n");
    }
	
    while(1)
    {
        //ioctl(fd,TIME_WRITE, time);

        ioctl(fd,TIME_READ,&rtc_tm);
        printf("\nsecond = %d\n",rtc_tm.second);
        printf("minute = %d\n",rtc_tm.minute);
        printf("hour = %d\n",rtc_tm.hour);
        printf("weekday = %d\n",rtc_tm.weekday);
        printf("date = %d\n",rtc_tm.date);
        printf("month = %d\n",rtc_tm.month);
        printf("year = %d\n",rtc_tm.year);

        sleep(1);

        //printf("di_value = 0x%x\n",arg->di_value);
        //printf("do_value = 0x%x\n",arg->do_value);
        //sleep(2);
    }
	
	close(fd);
}

