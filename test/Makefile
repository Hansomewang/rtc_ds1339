SHELL = /bin/sh

TARGET = rtc_test
CC = arm-hisiv100nptl-linux-gcc
#CC = gcc


OBJS = main.o



$(TARGET):$(OBJS)
	$(CC) -o $(TARGET) $(OBJS) $(CFLAGS) -lpthread


.PHONY: clean
clean:
	-rm $(TARGET) $(OBJS)


