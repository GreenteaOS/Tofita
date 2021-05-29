@echo off
color 0A
wmic cpu get name
wmic baseboard get product
wmic sounddev get name,PNPDeviceID
wmic PATH Win32_VideoController GET Caption,PNPDeviceID
wmic nic where "PNPDeviceID like '%%PCI%%'" get name,PNPDeviceID
pause
