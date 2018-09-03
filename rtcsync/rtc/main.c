/*
 * =====================================================================================
 *
 *       Filename:  main.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  2018-06-26 05:05:46 PM
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:   (), 
 *        Company:  
 *
 * =====================================================================================
 */

#include <stdio.h>
#include "ioctl.h"
#include "hi_rtc.h"

int main( int argc, char *argv )
{
	int a = HI_RTC_RD_TIME;
	int b = HI_RTC_SET_TIME;

	int c = HI_RTC_REG_SET;
	int d = HI_RTC_REG_READ;

	printf("531 %#x-%#x\r\n",a ,b  );
	printf("520 %#x-%#x\r\n",c ,d  );
	
}
