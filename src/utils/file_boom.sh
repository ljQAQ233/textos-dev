#!/usr/bin/env bash

if [ $# -ne 1 ]; then
    exit 1
fi

boom=$1

n1=1
n2=1024

for i in $(seq $n1) ; do
    mkdir -p $boom/$i
    for j in $(seq $n2) ; do
        touch $boom/$i/$j
    done
done