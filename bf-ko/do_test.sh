#!/bin/bash

reload() {
echo ----------------------------
echo ----- delete/insert ko -----
echo ----------------------------

bf=`lsmod | awk '$1 == "bf_filter" { print $1 }'`

if [ "$bf" = "bf_filter" ]; then
   echo  delete bf-filter
   rmmod bf-filter && {
       echo   inserting bf-filter 
       insmod bf-filter.ko && echo succeed
   }
else
   echo   inserting bf-filter
   insmod bf-filter.ko && echo succeed
fi

#lsmod 
#dmesg

}

echo ----------------------------
echo ---- building modules ------
echo ----------------------------

make && reload