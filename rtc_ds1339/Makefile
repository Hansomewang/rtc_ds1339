CC = arm-hisiv100nptl-linux-gcc
KERNELDIR:=../../linux-3.0.y/
PWD := $(shell pwd)


obj-m:=rtc_ds1339.o

modules:
	@$(MAKE) -C $(KERNELDIR) M=$(PWD) modules ARCH=arm CROSS_COMPILE=arm-hisiv100nptl-linux-


clean:
	@rm -rf *.o *.ko
