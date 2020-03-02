# Tofita

![Tofita Logo](docs/logo.png?raw=true)

All-new kernel for [Greentea OS](https://github.com/GreenteaOS) — [read Tofita wiki](https://greenteaos.github.io/Tofita/wiki/index.html)

[![Telegram Channel](https://img.shields.io/badge/Telegram-Greentea%20NEWS-blue.svg)](https://telegram.me/greenteaos_news)
[![Telegram Chatroom](https://img.shields.io/badge/Telegram-Greentea%20OS-blue.svg)](https://telegram.me/greenteaos)
[![Join the chat at Gitter!](https://img.shields.io/badge/Gitter-Join%20Chat-47B192.svg)](https://gitter.im/GreenteaOS/Lobby)
[![GPL License](https://img.shields.io/badge/License-GNU%20LGPLv3-green.svg?style=flat)](https://github.com/GreenteaOS/Tofita/blob/master/LICENSE)
[![GitHub Feed](https://img.shields.io/badge/GitHub-Feed-0f9d58.svg?style=flat)](https://t.me/greenteaos_github)
[![Nightly Builds](https://img.shields.io/badge/Nightly-Builds-ff69b4.svg?style=flat)](https://ci.appveyor.com/project/PeyTy/tofita/build/artifacts)
[![Language Top](https://img.shields.io/github/languages/top/GreenteaOS/Tofita.svg?colorB=green)](https://github.com/hexalang/hexa)
[![TODO](https://img.shields.io/badge/!-TODO-0f9d58.svg?style=flat)](https://github.com/GreenteaOS/Tofita/search?q=TODO&unscoped_q=TODO&type=Code)

Greentea OS is aimed to run `.exe` files natively, and to provide full operating system environment for that.

![Screenshot](https://raw.githubusercontent.com/GreenteaOS/Greentea/master/Images/screenshot.jpg)

## Features

 - UEFI 64-bit boot
 - 64-bit x86 oriented
 - Colorful graphics with transparency and double buffering
 - Quake-style terminal (hit `~` to show/hide)
 - Desktop GUI
 - PS/2 mouse
 - PS/2 keyboard
 - COM-port debug output
 - Initial RAM disk for LiveCD/LiveUSB and installer
 - Tested on:
   - VirtualBox 6.0+ `(use exactly "Mac OS X (64-bit)" preset)`
   - Intel Sandy Bridge `(socket 1155)`
   - AMD APU A10
   - Hyper-V `(use Generation 2, disable Secure Boot, no PS/2 support, read tools\pipe.js for more info)`
 - Monolithic performance-oriented kernel
 - Very low on resources, just 1 GB RAM minimum required
 - Isolation and containerization of everything
 - ACPI 2.0, CPUID
 - Exclusively only multi-core CPUs

## GreenteaOS (Tofita) system requirements

- 1 GB RAM
- Motherboard with UEFI, SATA (AHCI) or NVMe, PCIe and USB 2.0+
- Multicore modern 64-bit CPU (most AMD and Intel CPUs from 2009 and newer)
- [Read more](https://greenteaos.github.io/Tofita/wiki/cpus.html)

[![Give a star](docs/star.png?raw=true)
Give some attention for Tofita be noticed](https://github.com/GreenteaOS/Tofita/stargazers)

## Tools

- [`listen.js`](tools/listen.js) - listen to VirtualBox COM port in TCP socket mode
- [`pipe.js`](tools/pipe.js) - listen to VirtualBox\Hyper-V COM port in pipe mode
- [`gos-devlist.bat`](tools/gos-devlist.bat) - show useful device names and PCI IDs to gather user hardware stats

## Build

> **We are working to port this project to Hexa, so this repo currently has outdated code, don't build it**

Windows or [WHQ on Linux/macOS](https://www.winehq.org/) is required to run build scripts and tools

#### Nightly builds

If you don't want to build Tofita, you may [download latest automatic GitHub build](https://ci.appveyor.com/project/PeyTy/tofita/build/artifacts)

#### Teapot

Download or clone [Teapot repo](https://github.com/GreenteaOS/Teapot#download-latest-zip) and place alongside of Tofita repo:

```
.\Teapot
.\Tofita
```

You should have extracted `Teapot-master` and have folders like this `.\Teapot\LLVM-9.0.0`.

Teapot will take less space if cloned with limited depth of commit history:

```sh
git clone --recursive --depth 1 https://github.com/GreenteaOS/Tofita.git
git clone --recursive --depth 1 https://github.com/GreenteaOS/Teapot.git
```

#### Configure

Create `R:` ram disk about 1 GB (you may download a free [AMD Radeon™ RAMDisk](http://radeonramdisk.com/files/Radeon_RAMDisk_4_4_0_RC36.msi)) for faster builds and configure `config.json > tempFolder`.
Otherwise `C:` is used to store builds by default.

#### Install Hexa

You also need to install [Hexa compiler](https://github.com/hexalang/hexa#unstable).

Currently you will build Hexa manually too. Download [this](https://github.com/hexalang/hexa/archive/master.zip) and extract contents of `hexa-master` into `C:\Hexa`, so you have `C:\Hexa\hexa.cmd`.

Install [node.js LTS](https://nodejs.org/en/download/).

```sh
cd C:\hexa
node bootstrap.js hexa.json
node hexa-node.js hexa.json
node hexa-node.js hexa.json
```

Add `C:\Hexa` into your PATH.

#### Compile and generate disk image

```sh
cmd /c build
```

Or just open `build.bat` file with Explorer.

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
