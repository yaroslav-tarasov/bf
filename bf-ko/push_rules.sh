#!/bin/bash

echo "Put some rules to bf"
./netlink_cmd --new INPUT --srcport 53 --proto UDP
./netlink_cmd --new INPUT --srcport 666 --proto UDP
./netlink_cmd --new INPUT --srcport 800 --proto UDP
./netlink_cmd --new INPUT --srcport 300 --proto UDP
