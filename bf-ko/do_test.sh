#!/bin/bash

make && {
#if [ " lsmod | grep bf | awk `{print $1}`" -eq "bf_filter" ]; then
   echo "delete bf_filter"
   rmmod bf_filter
#if

insmod bf_filter.ko 

#lsmod 
#dmesg
}

