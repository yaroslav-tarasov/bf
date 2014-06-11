#!/bin/bash

make && {
#if [ " lsmod | grep bf | awk `{print $1}`" -eq "bf_filter" ]; then
   echo "delete bf_filter"
   rmmod bf-filter
#if

insmod bf-filter.ko 

#lsmod 
#dmesg
}

