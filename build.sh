#!/bin/bash

code="$PWD"
opts=-g
cd build > /dev/null
g++ $opts $code/code\main.c -o main.exe
cd $code > /dev/null
