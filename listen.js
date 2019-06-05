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

// Simple tool to read COM port data from TCP socket

// Exits when connection lost, so it may be useful from terminal

const net = require('net')

// Configuration
const command = '"C:\\Program Files\\Oracle\\VirtualBox\\VBoxManage" --nologo startvm Tofita'
const host = 'localhost'
const port = 1234

// Create server
const server = net.createServer(onClientConnected)

server.listen(port, host, function() {
	console.log('Server listening on %j', server.address())
	require('child_process').exec(command)
})

function onClientConnected(sock) {
	const client = sock.remoteAddress + ':' + sock.remotePort
	console.log('VirtualBox connected:', client)

	sock.on('data', (data) => {
		// Print
		process.stdout.write(data)
	})
	sock.on('close', () => {
		console.log('\r\nConnection from %s closed, exit', client)
		process.exit(0)
	})
	sock.on('error', (err) => {
		console.log('\r\nConnection %s error: %s', client, err.message)
	})
}
