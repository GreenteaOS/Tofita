#!/bin/sh
# chmod +x ./build.sh
# bash ./build.sh

# Install node.js somehow (from node.js website, not Ubuntu repos!)

# You need to build Hexa and add it to PATH
# ~/.bashrc
# export PATH="/home/USERNAME/hexa/:$PATH"

# sudo apt install xorriso clang-12 lld-12 nasm mtools qemu qemu-kvm
# Alternatively wine cmd /c build.bat

echo [32mWelcome to the Greentea OS! Support us at https://www.patreon.com/PeyTy and https://greenteaos.github.io/donate/
echo [0m

hexa build/hexa.json

if [ $? -ne 0 ]
then
echo [37m[41m[Exiting on error][0m
exit 1
fi

node build.js init asm uefi dll build iso

if [ $? -ne 0 ]
then
echo [37m[41m[Exiting on error][0m
exit 1
fi

# Check /tmp/tofita/ for output
# You can now try tools/qemu-vfat.sh
echo [36m[Success][0m
