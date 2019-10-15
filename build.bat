@echo off
color 0B
echo [Welcome to Tofita!]
cd %~dp0
cmd /c hexa build\hexa.json
IF %ERRORLEVEL% NEQ 0 (
  goto somethingbad
)
node build.js build
IF %ERRORLEVEL% NEQ 0 (
  goto somethingbad
)
color 0A
echo [Success]
::node build.js vbox
::node build.js qemu
::node tools\listen.js
::node tools\pipe.js
goto done
:somethingbad
color 0C
echo [Exiting on error]
:done
