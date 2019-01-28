@echo off
cmd /c hexa build\hexa.json
node build.js build qemu
