@echo off
cmd /c hexa build\hexa.json
node build.js build
::node build.js vbox
::node build.js qemu
::node tools\listen.js
::node tools\pipe.js
exit
