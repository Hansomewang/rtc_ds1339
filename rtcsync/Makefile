# this is 3531 
.PHONY: lib clean

LIBSRC:=$(wildcard *.c)
LIBOBJ:=$(LIBSRC:%.c=%.o)
APP_FLAGS:= -lpthread -lrt -lm
COMPLIER_FLAGS:= -g -Wall -I.

CC=arm-hisiv100nptl-linux-gcc
STRIP=arm-hisiv100nptl-linux-strip
#CC=arm-hisiv200-linux-gcc
#STRIP=arm-hisiv200-linux-strip
CLANE_HFLAGS=-I./rtc


# this will be set for each module
MLIB:=rtcsync

lib: $(MLIB)
$(MLIB) : $(LIBOBJ)
	$(CC) -o $@  $^ -lpthread -L.
	$(STRIP) $@

$(LIBOBJ) : %.o:%.c
	$(CC) $(CLANE_HFLAGS) $(COMPLIER_FLAGS) -c $^

clean:
	rm -f $(APPOBJ)
	rm -f $(LIBOBJ)
	rm -f $(APP)
	rm -f $(MLIB)
	rm -f tags
