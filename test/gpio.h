#ifndef _GPIO_H
#define _GPIO_H

typedef unsigned char Uint8;
typedef unsigned short Uint16;
typedef unsigned int Uint32;

#define TIME_READ   0x01
#define TIME_WRITE  0x03

typedef struct
{
    unsigned char  second;
    unsigned char  minute;
    unsigned char  hour;
    unsigned char  weekday;
    unsigned char  date;
    unsigned char  month;
    unsigned char  year;
//    unsigned char reg_addr;
} rtc_time;


#endif

