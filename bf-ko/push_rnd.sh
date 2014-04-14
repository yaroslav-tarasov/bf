#!/bin/bash

n=10 
if [ $# -gt 0 ]
then
    n=$1
else
    echo  $n "rules by default"
fi

echo "Put some rules to bf"


while [ "$n" -gt 0 ]
do
    ./netlink_cmd --new INPUT --srcport $(( ( RANDOM % 1500  )  + 1 ))  --proto UDP
    ./netlink_cmd --new INPUT --srcport $(( ( RANDOM % 1500  )  + 1 ))  --proto TCP
    echo $n 
    n=$(( $n - 1 ))
done


