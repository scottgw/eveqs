#!/bin/bash

for file in EIFGENs/*
do
    sed -i "s/\(\#include.*$\)/\1\n#undef eif_access\n#define eif_access(x) (x)\n/g" $file/F_code/E1/epattern.c
done
