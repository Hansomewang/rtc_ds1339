#include <linux/module.h>
#include <linux/errno.h>
#include <linux/miscdevice.h>
#include <linux/fcntl.h>

#include <linux/init.h>
#include <linux/delay.h>
#include <linux/proc_fs.h>
#include <linux/workqueue.h>

#include <asm/uaccess.h>
#include <asm/system.h>
#include <asm/io.h>

#include "rtc_ds1339.h" 
spinlock_t  gpioi2c_lock;

#define GPIO_12_BASE    0x20210000
#define I2C_ADDR    0xD0

#define GPIO_DIR      IO_ADDRESS(GPIO_12_BASE + 0x400)
#define SCL_SHIFT_NUM   0x5
#define SDA_SHIFT_NUM   0x4

#define SCL                 (1 << 5)    /* GPIO12 0_5 */
#define SDA                 (1 << 4)    /* GPIO12 0_4 */
#define GPIO_I2C_SCL_REG    IO_ADDRESS(GPIO_12_BASE + 0x80)  /* 0x80 */
#define GPIO_I2C_SDA_REG    IO_ADDRESS(GPIO_12_BASE + 0x40)  /* 0x40 */

#define GPIO_I2C_SCLSDA_REG IO_ADDRESS(GPIO_12_BASE + 0xc0)  /* need check */

#define HW_REG(reg)         *((volatile unsigned int *)(reg))
#define DELAY(us)           time_delay_us(us)


/* 
 * I2C by GPIO simulated  clear 0 routine.
 *
 * @param whichline: GPIO control line
 *
 */
static void i2c_clr(unsigned char whichline)
{
	unsigned char regvalue;
	
	if(whichline == SCL)
	{
		regvalue = HW_REG(GPIO_DIR);
		regvalue |= SCL;
		HW_REG(GPIO_DIR) = regvalue;
		
		HW_REG(GPIO_I2C_SCL_REG) = 0;
		return;
	}
	else if(whichline == SDA)
	{
		regvalue = HW_REG(GPIO_DIR);
		regvalue |= SDA;
		HW_REG(GPIO_DIR) = regvalue;
		
		HW_REG(GPIO_I2C_SDA_REG) = 0;
		return;
	}
	else if(whichline == (SDA|SCL))
	{
		regvalue = HW_REG(GPIO_DIR);
		regvalue |= (SDA|SCL);
		HW_REG(GPIO_DIR) = regvalue;
		
		HW_REG(GPIO_I2C_SCLSDA_REG) = 0;
		return;
	}
	else
	{
		printk("Error input.\n");
		return;
	}
}

/* 
 * I2C by GPIO simulated  set 1 routine.
 *
 * @param whichline: GPIO control line
 *
 */
static void  i2c_set(unsigned char whichline)
{
	unsigned char regvalue;
	
	if(whichline == SCL)
	{
		regvalue = HW_REG(GPIO_DIR);
		regvalue |= SCL;
		HW_REG(GPIO_DIR) = regvalue;
		
		HW_REG(GPIO_I2C_SCL_REG) = SCL;
		return;
	}
	else if(whichline == SDA)
	{
		regvalue = HW_REG(GPIO_DIR);
		regvalue |= SDA;
		HW_REG(GPIO_DIR) = regvalue;
		
		HW_REG(GPIO_I2C_SDA_REG) = SDA;
		return;
	}
	else if(whichline == (SDA|SCL))
	{
		regvalue = HW_REG(GPIO_DIR);
		regvalue |= (SDA|SCL);
		HW_REG(GPIO_DIR) = regvalue;
		
		HW_REG(GPIO_I2C_SCLSDA_REG) = (SDA|SCL);
		return;
	}
	else
	{
		printk("Error input.\n");
		return;
	}
}

/*
 *  delays for a specified number of micro seconds rountine.
 *
 *  @param usec: number of micro seconds to pause for
 *
 */
 // FPGA  APB :  25M
 // ASIC  APB : 155M
 //  ·­×ª5±¶
 void time_delay_us(unsigned int usec)
{
	volatile int i,j;
	/*
	//FPGA: 25MHZ
	for(i=0;i<usec * 5;i++)
	{
		for(j=0;j<47;j++)
		{;}
	}

    */
    //ASIC: 155MHZ   
    //AP = 155/25 = 6.2
    for(i=0;i<usec * 2;i++)
	{
		for(j=0;j<50*6;j++)
		{;}
	}
}

/* 
 * I2C by GPIO simulated  read data routine.
 *
 * @return value: a bit for read 
 *
 */
 
static unsigned char i2c_data_read(void)
{
	unsigned char regvalue;
	
	regvalue = HW_REG(GPIO_DIR);
	regvalue &= (~SDA);
	HW_REG(GPIO_DIR) = regvalue;
	DELAY(1);
		
	regvalue = HW_REG(GPIO_I2C_SDA_REG);
	if((regvalue&SDA) != 0)
		return 1;
	else
		return 0;
}



/*
 * sends a start bit via I2C rountine.
 *
 */
static void i2c_start_bit(void)
{
        DELAY(1);
        i2c_set(SDA | SCL);
        DELAY(1);
        i2c_clr(SDA);
        DELAY(1);
}

/*
 * sends a stop bit via I2C rountine.
 *
 */
static void i2c_stop_bit(void)
{
        /* clock the ack */
        DELAY(1);
        i2c_set(SCL);
        DELAY(1); 
        i2c_clr(SCL);  

        /* actual stop bit */
        DELAY(1);
        i2c_clr(SDA);
        DELAY(1);
        i2c_set(SCL);
        DELAY(1);
        i2c_set(SDA);
        DELAY(1);
}

/*
 * sends a character over I2C rountine.
 *
 * @param  c: character to send
 *
 */
static void i2c_send_byte(unsigned char c)
{
    int i;
    local_irq_disable();
    for (i=0; i<8; i++)
    {
        DELAY(1);
        i2c_clr(SCL);
        DELAY(1);

        if (c & (1<<(7-i)))
            i2c_set(SDA);
        else
            i2c_clr(SDA);

        DELAY(1);
        i2c_set(SCL);
        DELAY(1);
        i2c_clr(SCL);
    }
    DELAY(1);
   // i2c_set(SDA);
    local_irq_enable();
}

/*  receives a character from I2C rountine.
 *
 *  @return value: character received
 *
 */
static unsigned char i2c_receive_byte(void)
{
    int j=0;
    int i;
    unsigned char regvalue;

    local_irq_disable();
    for (i=0; i<8; i++)
    {
        DELAY(1);
        i2c_clr(SCL);
        DELAY(1);
        i2c_set(SCL);
        
        regvalue = HW_REG(GPIO_DIR);
        regvalue &= (~SDA);
        HW_REG(GPIO_DIR) = regvalue;
        DELAY(1);
        
        if (i2c_data_read())
            j+=(1<<(7-i));

        DELAY(1);
        i2c_clr(SCL);
    }
    local_irq_enable();
    DELAY(1);
   // i2c_clr(SDA);
   // DELAY(1);

    return j;
}

/*  receives an acknowledge from I2C rountine.
 *
 *  @return value: 0--Ack received; 1--Nack received
 *          
 */
static int i2c_receive_ack(void)
{
    int nack;
    unsigned char regvalue;
    
    DELAY(1);
    
    regvalue = HW_REG(GPIO_DIR);
    regvalue &= (~SDA);
    HW_REG(GPIO_DIR) = regvalue;
    
    DELAY(1);
    i2c_clr(SCL);
    DELAY(1);
    i2c_set(SCL);
    DELAY(1);
    
    

    nack = i2c_data_read();

    DELAY(1);
    i2c_clr(SCL);
    DELAY(1);
  //  i2c_set(SDA);
  //  DELAY(1);

    if (nack == 0)
        return 1; 

    printk(" not receive ack!\n");
    return 0;
}

#if 0
static void i2c_send_ack(void)
{
    DELAY(1);
    i2c_clr(SCL);
    DELAY(1);
    i2c_set(SDA);
    DELAY(1);
    i2c_set(SCL);
    DELAY(1);
    i2c_clr(SCL);
    DELAY(1);
    i2c_clr(SDA);
    DELAY(1);
}
#endif

unsigned char gpio_i2c_read(unsigned char address)
{
    int rxdata;

    spin_lock(&gpioi2c_lock);
    
    i2c_start_bit();
    i2c_send_byte( I2C_ADDR );
    i2c_receive_ack();
    i2c_send_byte(address);
    i2c_receive_ack();   
    i2c_start_bit();
    i2c_send_byte( I2C_ADDR | 1);
    i2c_receive_ack();
    rxdata = i2c_receive_byte();
    //i2c_send_ack();
    i2c_stop_bit();

    spin_unlock(&gpioi2c_lock);
    return rxdata;
}


void gpio_i2c_write( unsigned char address, unsigned char data)
{
    spin_lock(&gpioi2c_lock);
    
    i2c_start_bit();
    i2c_send_byte( I2C_ADDR );
    i2c_receive_ack();
    i2c_send_byte(address);
    i2c_receive_ack();
    i2c_send_byte(data); 
   // i2c_receive_ack();//add by hyping for tw2815
    i2c_stop_bit();

   spin_unlock(&gpioi2c_lock);
}

int get_time( rtc_time *time )
{
    time->second =  gpio_i2c_read( Second );
    time->second = (time->second & 0x0f) + (time->second >> 4)*10;

    time->minute = gpio_i2c_read( Minute );
    time->minute = (time->minute & 0x0f) + (time->minute >> 4)*10;

    time->hour = gpio_i2c_read( Hours );
    time->hour = ( time->hour & 0x0f ) + ((time->hour >> 4) & 0x03)*10;

    time->weekday = gpio_i2c_read( Weekday );
    time->weekday = time->weekday & 0x07;

    time->date = gpio_i2c_read( Date );
    time->date = (time->date & 0x0f) + ( time->date >> 4)*10;

    time->month = gpio_i2c_read( Month );
    time->month = ( time->month & 0x0f ) + (( time->month >> 4) & 0x01)*10;

    time->year = gpio_i2c_read( Year );
    time->year = ( time->year & 0x0f ) + ( time->year >> 4 )*10;

    return 0;
}

int set_time( rtc_time *time )
{
    unsigned char high , low;
    unsigned char tmp;

    high = time->second / 10;
    low = time->second % 10;
    time->second = (high << 4) | low; 
    gpio_i2c_write( Second, time->second );

    high = time->minute / 10;
    low = time->minute % 10;
    time->minute = (high << 4) | low;
    gpio_i2c_write( Minute, time->minute );

    high = time->hour / 10;
    low = time->hour % 10;
    tmp = gpio_i2c_read( Hours );
    tmp = tmp >> 6;
    time->hour = (tmp << 6) | ((high << 4) & 0x30) | low;
    gpio_i2c_write( Hours, time->hour );

    time->weekday = time->weekday & 0x07;
    gpio_i2c_write( Weekday, time->weekday );

    high = time->date / 10;
    low = time->date % 10;
    time->date = (high << 4) | low;
    gpio_i2c_write( Date, time->date );

    tmp = gpio_i2c_read(Month);
    tmp = tmp >> 7;
    high = time->month / 10;
    low = time->month % 10;
    time->month = (tmp << 7) | ((high << 4) & 0x10) | low;
    gpio_i2c_write( Month, time->month );

    high = time->year / 10;
    low = time->year % 10;
    time->year = (high << 4) | low;
    gpio_i2c_write( Year, time->year );

    return 0;
}

int gpioi2c_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    unsigned int val;
    rtc_time time;
	
	char device_addr, reg_addr;
	short reg_val;
	
	
	switch(cmd)
	{
		case TIME_READ:
            get_time( &time );
            if( copy_to_user((void *)arg, &time, sizeof(rtc_time) ))
                printk("copy to user err\n");
			
			break;
		
		case TIME_WRITE:
            if( copy_from_user(&time,(void *)arg, sizeof(rtc_time)) )
                printk("copy from user err\n");
            printk("time.second = %d\n",time.second);
            set_time( &time );
			break;		
	
		default:
			return -1;
	}
    return 0;
}

int gpioi2c_open(struct inode * inode, struct file * file)
{
    return 0;
}
int gpioi2c_close(struct inode * inode, struct file * file)
{
    return 0;
}


static struct file_operations gpioi2c_fops = {
    .owner      = THIS_MODULE,
    //.ioctl      = gpioi2c_ioctl,
    .unlocked_ioctl = gpioi2c_ioctl,
    .open       = gpioi2c_open,
    .release    = gpioi2c_close
};


static struct miscdevice gpioi2c_dev = {
   .minor		= MISC_DYNAMIC_MINOR,
   .name		= "rtc_time",
   .fops  = &gpioi2c_fops,
};

static int __init gpio_i2c_init(void)
{
    int ret;
    //unsigned int reg;
    
    ret = misc_register(&gpioi2c_dev);
    if(0 != ret)
    	return -1;
        
#if 1         
    //printk(KERN_INFO OSDRV_MODULE_VERSION_STRING "\n");            
    //reg = HW_REG(SC_PERCTRL1);
    //reg |= 0x00004000;
    //HW_REG(SC_PERCTRL1) = reg;
    i2c_set(SCL | SDA);
#endif    

      spin_lock_init(&gpioi2c_lock);
    return 0;    
}

static void __exit gpio_i2c_exit(void)
{
    misc_deregister(&gpioi2c_dev);
}


module_init(gpio_i2c_init);
module_exit(gpio_i2c_exit);

#ifdef MODULE
//#include <linux/compile.h>
#endif
//MODULE_INFO(build, UTS_VERSION);
MODULE_LICENSE("GPL");

