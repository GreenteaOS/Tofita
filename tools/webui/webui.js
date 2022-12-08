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

// Read listen.js for more info

// Configuration
const vmname = 'Tofita'
const command = '"C:\\Program Files\\Oracle\\VirtualBox\\VBoxManage" --nologo startvm ' + vmname
const host = 'localhost'
const port = 1234

// Web UI
const portWebUI = 4321
console.log('Open Web UI at http://localhost:4321/index.html')

// Imports
const fs = require('fs')
const net = require('net')
const http = require('http')

// Globals
const buffer = []

//create a server object:
http.createServer(function(req, res) {
	const url = req.url.trim().toLowerCase()
	if (url == '/' || url == '/index.html') {
		res.writeHead(200, { 'Content-Type': 'text/html' });
		res.write(fs.readFileSync('webui.html'));
		res.end();
		return
	}

	if (url == '/favicon.ico') {
		res.writeHead(200, { 'Content-Type': 'image/x-icon' });
		res.write(fs.readFileSync('..\\..\\assets\\autorun.ico'));
		res.end();
		return
	}

	if (url == '/buffer.json') {
		res.writeHead(200, {
			'Content-Type': 'application/json',
			'Transfer-Encoding': 'chunked'
		});

		function send() {
			if (buffer.length == 0) {
				res.write('')
				return setTimeout(send, 111)
			}

			res.write(JSON.stringify(buffer.join('')));
			buffer.length = 0
			res.end();
		}
		setTimeout(send, 111)
		return
	}

	if (true) {
		res.writeHead(404, { 'Content-Type': 'text/html' });
		res.write(req.url + ' not found');
		res.end();
		return
	}
}).listen(portWebUI);

// Create server
const server = net.createServer(onClientConnected)

server.listen(port, host, function() {
	console.log('Server listening on %j', server.address())
	//require('child_process').exec(command)
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
		buffer.push(s.join(''))
	})
	sock.on('close', () => {
		console.log('\r\nConnection from %s closed', client)
	})
	sock.on('error', (err) => {
		console.log('\r\nConnection %s error: %s', client, err.message)
	})
}
