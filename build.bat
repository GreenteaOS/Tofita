@echo off
color 0B
echo [Welcome to Tofita!]
cd /d %~dp0
cmd /c hexa build\hexa.json
IF %ERRORLEVEL% NEQ 0 (
  goto somethingbad
)
::node build.js clang-format
node build.js build
IF %ERRORLEVEL% NEQ 0 (
  goto somethingbad
)
color 0A
echo [Success]
::node build.js vbox
::node tools\vbox.js
::node tools\listen.js
::node tools\pipe.js
goto done
:somethingbad
color 0C
echo [Exiting on error]
:done
:: Clear error code
set errorlevel=0
exit /b 0
