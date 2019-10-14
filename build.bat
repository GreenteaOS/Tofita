@echo off
cmd /c hexa build\hexa.json
IF %ERRORLEVEL% NEQ 0 (
  echo "[Exiting on error]"
  exit
)
node build.js build
IF %ERRORLEVEL% NEQ 0 (
  echo "[Exiting on error]"
  exit
)
::node build.js vbox
::node build.js qemu
::node tools\listen.js
::node tools\pipe.js
exit
