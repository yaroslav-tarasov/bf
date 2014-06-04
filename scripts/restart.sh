#!/bin/bash

bfs_pid=`ps -eo comm,pid | awk '$1 == "bf-service" { print $2 }'`

if [ $bfs_pid -gt 0 ]
then
   service bfservice stop
   while [ -d '/proc/$bfs_pid' ]
   do
    echo "wait for stop"  $bfs_pid
   done
   
fi

cd /home/sintez/bf/bf-ko
./do_test.sh && service bfservice start 
cd /home/sintez/bin
#./bfctl --append INPUT --srcport 53 --proto UDP





 
