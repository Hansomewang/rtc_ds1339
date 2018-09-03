#!/bin/bash 

make 

if [ -f "rtc_test" ];then
    echo "cp rtc_test /home/wj/share/"
    cp rtc_test /home/wj/share/
else
    echo "no file"
fi


