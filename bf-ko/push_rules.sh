#!/bin/bash

echo "Put some rules to bf"
./bf_cmd --append INPUT --sport 53 --proto UDP
./bf_cmd --append INPUT --sport 666 --proto UDP
./bf_cmd --append INPUT --sport 800 --proto UDP
./bf_cmd --append INPUT --sport 300 --proto UDP
