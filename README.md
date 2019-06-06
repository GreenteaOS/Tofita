# Tofita

![Tofita Logo](docs/logo.png?raw=true)

All-new kernel for [Greentea OS](https://github.com/GreenteaOS)


[![Telegram Channel](https://img.shields.io/badge/Telegram-Greentea%20NEWS-blue.svg)](https://telegram.me/greenteaos_news)
[![Telegram Chatroom](https://img.shields.io/badge/Telegram-Greentea%20ENG-blue.svg)](https://telegram.me/greenteaos)
[![Join the chat at Gitter!](https://img.shields.io/badge/Gitter-Join%20Chat-47B192.svg)](https://gitter.im/GreenteaOS/Lobby)
[![GPL License](https://img.shields.io/badge/License-GNU%20LGPLv3-green.svg?style=flat)](https://github.com/GreenteaOS/Tofita/blob/master/LICENSE)
[![GitHub Feed](https://img.shields.io/badge/GitHub-Feed-0f9d58.svg?style=flat)](https://t.me/greenteaos_github)
[![Nightly Builds](https://img.shields.io/badge/Nightly-Builds-ff69b4.svg?style=flat)](https://ci.appveyor.com/project/PeyTy/TODO/build/artifacts)
[![Language Top](https://img.shields.io/github/languages/top/GreenteaOS/Tofita.svg?colorB=green)](https://github.com/hexalang/hexa)

Greentea OS is aimed to run `.exe` files natively, and to provide full operating system environment for that.

## Features

 - UEFI boot
 - 64-bit x86
 - Colorful graphics with transparency
 - PS/2 mouse
 - PS/2 keyboard
 - COM-port debug output
 - Initial RAM disk for LiveCD/LiveUSB and installer
 - Tested on:
   - QEMU 3.0.0+ `(use OVMF to emulate UEFI)`
   - VirtualBox 6.0+ `(use macOS 64-bit mode, set COM port to pipe \\.\pipe\tofita-com1 deselect "Connect to existing")`
   - Hyper-V `(use Generation 2, disable Secure Boot, no PS/2 support, read pipe.js for more info)`
   - Intel Sandy Bridge `(socket 1155)`
 - Monolithic performance-oriented kernel
 - ACPI

[![Give a star](docs/star.png?raw=true)
Give some attention for Tofita be noticed](https://github.com/GreenteaOS/Tofita/stargazers)

> Note: This system is going to be coded exclusively in [@hexalang](https://github.com/hexalang), without any C/C++ code.

## Tools

- [`listen.js`](listen.js) - listen to VirtualBox COM port in TCP socket mode `(localhost:1234)`
- [`pipe.js`](pipe.js) - listen to VirtualBox\Hyper-V COM port in pipe mode `(\\.\pipe\tofita-com1)`

## Build

Currently Windows 10 with WSL is required. This requirement will be relaxed (to support winehq and any Windows version, no WSL). You may try to build under Linux by remaking build scripts.

#### Teapot

Download or clone [Teapot repo](https://github.com/GreenteaOS/Teapot#download-latest-zip) and place alongside of Tofita repo:

```
.\Teapot
.\Tofita
```

Teapot will take less space if cloned with limited depth of commit history:

```sh
git clone --recursive --depth 1 https://github.com/GreenteaOS/Teapot.git
```

#### WSL

```sh
sudo apt install clang-6.0 nasm mtools xorriso
```

#### CMD

Install [node.js](https://nodejs.org/en/download/).

Create `R:\` ram disk or just configure `config.json > tempFolder`.

You also need [Hexa compiler](https://github.com/hexalang/hexa#unstable).

```sh
cmd /c build
```

## License

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, version 3 of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

[More information about legal issues](https://github.com/GreenteaOS/Greentea/blob/master/README.md#license)
