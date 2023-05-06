#!/bin/bash

echo "Compiling module"
make
sleep 1

echo "Inserting module"
insmod vader.ko
sleep 1

echo "Running virt checks"
systemd-detect-virt
sleep 1
python3 ../user_level_detect.py
sleep 1

echo "Removing module"
rmmod vader
sleep 1

echo "Testing CPUID"
./cpuid_test
sleep 3

echo "Cleaning up..."
make clean
sleep 1

echo "Logs..."
dmesg


