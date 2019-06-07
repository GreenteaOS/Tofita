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

// Simple tool to read COM port data from pipe
// `node pipe.js`

// Reconnects automatically after VM reboots, so you don't have to restart this script

// RUN ONCE AND KEEP IT RUNNING FOREVER (keeps CPU usage at low)

// THIS SCRIPT MAY REQUIRE ADMIN RIGHTS TO LISTEN FOR HYPER-V PIPES
/* Hyper-V

Open PowerShell as Administrator

Set-VMComPort -VMName Tofita -Path \\.\pipe\tofita-com1 -Number 1
Set-VMComPort -VMName Tofita -Path \\.\pipe\tofita-com2 -Number 2
Get-VMComPort -VMName Tofita

*/

const PIPE_NAME = "tofita-com1"
const PIPE_PATH = "\\\\.\\pipe\\" + PIPE_NAME

// Disallow breaking terminal with special chars
const allowed = 'qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM1234567890`~!@#$%^&*()_+-=[]{}\\|;\'",./<>?:\t\r\n '

var client = require('net').connect(PIPE_PATH, function() {})

// Avoid nodejs crashes:
const domain = require('domain').create()
domain.on('error', function(err) {
	console.error(err)
})

function reconnect() {
	client.removeAllListeners()

	client.on('data', function(data) {
		let s = []
		for (let i = 0; i < data.length; i++) {
			if (data[i] == 0) continue
			const char = String.fromCharCode(data[i])
			if (allowed.indexOf(char) != -1)
				s.push(char)
		}
		process.stdout.write(s.join(''), 'ascii')
	})

	client.on('end', function() {})
	client.on('close', function() {
		reconnect()
	})

	client.on('error', function() {
		reconnect()
	})

	setTimeout(() => {
		client.connect(PIPE_PATH)
	}, 1)
}

reconnect()
