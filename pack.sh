#!/bin/sh
exe="ziwi"
des="/home/lux/Desktop/Ziwi/lib/Linux"
deplist=$(ldd $exe | awk '{if (match($3, "/")){ printf("%s "),$3}}')
cp $deplist $des
