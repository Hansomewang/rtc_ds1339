
#ifndef _GPIO_I2C_H
#define _GPIO_I2C_H


#define TIME_READ   0x01
#define TIME_WRITE  0x03
typedef unsigned char		byte;

unsigned char gpio_i2c_read( unsigned char address);
void gpio_i2c_write( unsigned char address, unsigned char value);
byte siiReadSegmentBlockEDID(byte SlaveAddr, byte Segment, byte Offset, byte *Buffer, byte Length);

#define Second     0x00
#define Minute     0x01
#define Hours       0x02
#define Weekday     0x03
#define Date        0x04
#define Month       0x05
#define Year        0x06


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

