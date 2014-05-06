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
    ./bfctl --append INPUT --sport $(( ( RANDOM % 1500  )  + 1 ))  --proto UDP
    ./bfctl --append INPUT --sport $(( ( RANDOM % 1500  )  + 1 ))  --proto TCP
    ./bfctl --append OUTPUT --sport $(( ( RANDOM % 1500  )  + 1 ))  --proto UDP
    ./bfctl --append OUTPUT --sport $(( ( RANDOM % 1500  )  + 1 ))  --proto TCP

    echo $n 
    n=$(( $n - 1 ))
done


