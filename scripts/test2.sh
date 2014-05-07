#!/bin/bash
clear

./push_rnd.sh 200
./bfctl --append INPUT --proto UDP --dport 18000
./push_rnd.sh 200


