#!/bin/bash

echo "Put some rules to bf"
./bfctl --append OUTPUT --sport 53 --proto UDP
./bfctl --append OUTPUT --sport 666 --proto UDP
./bfctl --append OUTPUT --sport 800 --proto UDP
./bfctl --append OUTPUT --sport 300 --proto UDP
