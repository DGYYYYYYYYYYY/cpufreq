#!/bin/sh
riscv64-unknown-linux-gnu-gcc -O3 -o mhz mhz.c lib_timing.c lib_stats.c getopt.c -static -lm

