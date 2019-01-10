@echo off
:: Configure
@set R=R:
@set QEMU=C:\qemu

:: Build Kernel
bash build.sh

:: VirtualBox
::"C:\Program Files\Oracle\VirtualBox\VBoxManage" modifyvm Tofita --bioslogodisplaytime 0
::"C:\Program Files\Oracle\VirtualBox\VBoxManage" --nologo startvm Tofita

:: Run
%QEMU%\qemu-system-x86_64 ^
	-display sdl ^
	-show-cursor ^
	-monitor stdio ^
	-net none ^
	-boot menu=off ^
	-pflash OVMF-20160813.fd ^
	-serial file:%R%\qemu-serial.log ^
	-cdrom %R%\tofita\uefi.iso ^
	-s ^
	-no-reboot ^
	-vga std ^
	-smp 4 ^
	-m 128M ^
	-net nic,macaddr=52-54-00-B3-47-55,model=rtl8139
