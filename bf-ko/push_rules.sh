#!/bin/bash

echo "Put some rules to bf"
./bf_cmd --new INPUT --srcport 53 --proto UDP
./bf_cmd --new INPUT --srcport 666 --proto UDP
./bf_cmd --new INPUT --srcport 800 --proto UDP
./bf_cmd --new INPUT --srcport 300 --proto UDP
