#!/bin/sh
for i in 1 3 5 7 9
do
    for type in "TINY" "STD"
    do
        echo "$i ${type}"
        g++ -Wall -Wextra -pedantic -Os -DBENCHMARK_${type} -DBENCHMARK_MAX=100000000 -DBENCHMARK_WHICH=$i main.cc && time -p ./a.out
    done
done
