// The Tofita Kernel
// Copyright (C) 2019  Oleg Petrenko
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

const fs = require('fs')

const config = JSON.parse(fs.readFileSync('config.json').toString())
const tasks = process.argv.slice()
tasks.shift()
tasks.shift()

if (tasks.length == 0) console.log('No tasks defined')

for (const task of tasks) {
	switch (task) {
		case 'build':
			{
				require('child_process').execSync(
					'node assets\\assets.js', { stdio: 'inherit' }
				)
				// Assembly
				const asm = ['kernel\\tofita.asm']
				for (const as of asm)
					require('child_process').execSync(
						config.teapotFolder + '\\nasm-2.14.02\\nasm -f elf64 -o ' + config.tempFolder + '\\tofita\\' + as.split('\\')[1] + '.o ' + as, { stdio: 'inherit' }
					)
				break;
				require('child_process').execSync(
					'bash build.sh', { stdio: 'inherit' }
				)
			};
			break;
		case 'qemu':
			{
				require('child_process').execSync(
					[
						config.qemuFolder + '\\qemu-system-x86_64',
						'-display sdl',
						'-show-cursor',
						'-monitor stdio',
						'-boot menu=off',
						'-pflash OVMF-20160813.fd',
						'-serial stdio',
						'-cdrom ' + config.tempFolder + '\\tofita\\uefi.iso',
						'-s',
						'-no-reboot',
						'-vga std',
						'-smp 4',
						'-m 1024M',
						'-net nic,macaddr=52-54-00-B3-47-55,model=rtl8139'
					].join(' '), { stdio: 'inherit' }
				)
			};
			break;
		case 'vbox':
			{
				require('child_process').execSync(
					'node listen.js', { stdio: 'inherit' }
				)
			};
			break;
		default:
			console.log('Unknown task', task)
	}
}
