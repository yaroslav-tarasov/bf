#!/bin/bash

n=10 
if [ $# -gt 0 ]
then
    n=$1
else
    echo  $n "rules by default"
fi

echo "Put some rules to bf"

base_port=45000

while [ "$n" -gt 0 ]
do
    ./bf_cmd --append INPUT --sport $(( $base_port - $n ))  --proto UDP
    ./bf_cmd --append INPUT --sport $(( $base_port - $n  ))  --proto TCP
    echo $n 
    n=$(( $n - 1 ))
done


