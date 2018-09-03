#!/bin/bash 

make 

if [ -f "rtc_ds1339.ko" ];then
    echo "cp rtc_ds1339.ko /home/wj/share/"
    cp rtc_ds1339.ko /home/wj/share/
else
    echo "no file"
fi


