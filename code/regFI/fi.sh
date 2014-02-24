#!/bin/bash
#insmod /home/linye/reg-new/reg-inject.ko
echo start > /proc/reg_inject/DRControl
echo 1 > /proc/reg_inject/pid
echo 0 > /proc/reg_inject/faulttype
echo $1 > /proc/reg_inject/faultlocation
echo 1 > /proc/reg_inject/faultinterval
sleep 3
#date >> log.txt
cat /proc/reg_inject/output >> log.txt
echo stop > /proc/reg_inject/DRControl
