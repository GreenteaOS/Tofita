:: Configure
@set R=R:\
@set QEMU=C:\qemu

:: Build Kernel
bash build.sh

:: Run
%QEMU%\qemu-system-x86_64 ^
	-display sdl ^
	-show-cursor ^
	-monitor stdio ^
	-net none ^
	-boot menu=off ^
	-pflash OVMF-20160813.fd ^
	-serial file:%R%qemu-serial.log ^
	-cdrom %R%uefi.iso ^
	-s ^
	-no-reboot ^
	-vga std ^
	-smp 4 ^
	-m 128M ^
	-net nic,macaddr=52-54-00-B3-47-55,model=rtl8139
