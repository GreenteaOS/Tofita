// The Tofita Kernel
// Copyright (C) 2020 Oleh Petrenko
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

// Simple tool to read COM port data from TCP socket
// In VirtualBox VM settings:
// General -> Name `Tofita` or set vmname var
// Serial Ports -> Port 1
//                        -> Enable "Enable Serial Port"
//                        -> Port Mode "TCP"
//                        -> Enable "Connect to existing pipe/socket"
//                        -> Path localhost:1234 or edit host/port vars
// `node listen.js`
// Runs VirtualBox automatically

// Exits when connection lost, so it may be useful from terminal

const net = require('net')

// Configuration
const vmname = 'Tofita'
const command = '"C:\\Program Files\\Oracle\\VirtualBox\\VBoxManage" --nologo startvm ' + vmname
const host = 'localhost'
const port = 1234

// Create server
const server = net.createServer(onClientConnected)

server.listen(port, host, function() {
	console.log('Server listening on %j', server.address())
	require('child_process').exec(command)
})

// Disallow breaking terminal with special chars
const allowed = 'qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM1234567890`~!@#$%^&*()_+-=[]{}\\|;\'",./<>?:\t\r\n '

function onClientConnected(sock) {
	const client = sock.remoteAddress + ':' + sock.remotePort
	console.log('VirtualBox connected:', client)

	sock.on('data', (data) => {
		let s = []
		for (let i = 0; i < data.length; i++) {
			if (data[i] == 0) continue
			const char = String.fromCharCode(data[i])
			if (allowed.indexOf(char) != -1)
				s.push(char)
		}
		process.stdout.write(s.join(''), 'ascii')
	})
	sock.on('close', () => {
		console.log('\r\nConnection from %s closed, exit', client)
		process.exit(0)
	})
	sock.on('error', (err) => {
		console.log('\r\nConnection %s error: %s', client, err.message)
	})
}
