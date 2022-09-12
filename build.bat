@echo off
echo [32mWelcome to the Greentea OS! Support us at https://www.patreon.com/PeyTy and https://greenteaos.github.io/donate/
echo [0m
cd /d %~dp0

cmd /c hexa build\hexa.json
IF %ERRORLEVEL% NEQ 0 (
  goto somethingbad
)

cmd /c hexa boot\hexa.json
IF %ERRORLEVEL% NEQ 0 (
  goto somethingbad
)

::debug
node build.js init-or-clean asm efi dll kernel ramdisk iso
::
IF %ERRORLEVEL% NEQ 0 (
  goto somethingbad
)
echo [36m[Success][0m
ATTRIB +S .
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
