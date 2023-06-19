@echo off
echo [32mWelcome to the Greentea OS! Support us at https://www.patreon.com/PeyTy and https://greenteaos.github.io/donate/
echo [0m
cd /d %~dp0
::TODO if not exists Greentea You seems not downloaded repo... Can be downloaded at url
::TODO if not exists Teapot You seems not downloaded repo...
::TODO hexa--is-present , if err code get hexa at url

:: Balanced
:: powercfg /setactive 381b4222-f694-41f0-9685-ff5bb260df2e

:: Build the builder itself
cmd /c hexa build\hexa.json
IF %ERRORLEVEL% NEQ 0 (
  goto somethingbad
)
:: TODO ! hexa --deps(name+date)path.txt to print for deps while compile, so
:: we can compare dates for cashes and rebuild only required projects

::debug
node build.js init-or-clean asm efi dll kernel ramdisk iso
::
IF %ERRORLEVEL% NEQ 0 (
  goto somethingbad
)
echo [36m[Success][0m
ATTRIB +S .

:: Start VMs or tools
::tools\qemu-vfat.bat
::cd /d "C:\Program Files\Oracle\VirtualBox"
::VBoxManage startvm "Tofita"

:: Eco
:: powercfg /setactive 4353c124-51fe-4f9b-b6b2-0d3f94691a20

::node build.js vbox
::node tools\vbox.js
::node tools\listen.js
::node tools\pipe.js

goto done
:somethingbad
echo [37m[41m[Exiting on error][0m
:done
:: Clear error code
set errorlevel=0
exit /b 0
