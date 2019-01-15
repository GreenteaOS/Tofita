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

const assets = JSON.parse(fs.readFileSync('assets/assets.json').toString())
console.log(assets)

// Compute RAM disk size
let ramDiskSize = 0
for (const asset of assets) {
	ramDiskSize += fs.statSync('assets\\' + asset).size
}

ramDiskSize += 4 // uint32_t ramDiskChecksum;
ramDiskSize += 4 // uint32_t ramDiskSize;
ramDiskSize += 4 // uint32_t assetsCount;
ramDiskSize += ( //struct assetDefinition
	//{
	256 + //	uint8_t path[256];
	4 + //	    uint32_t size;
	4 //	    uint32_t offsetFromRamDiskFirstByte; // NOT relative to `data`
	//};
) * assets.length
//uint8_t* data;

// Round upto 4 bytes
ramDiskSize = Math.ceil(ramDiskSize / 4) * 4
console.log('ramDiskSize is', ramDiskSize, 'bytes')

const buffer = Buffer.alloc(ramDiskSize) // zeroed
buffer.writeUInt32LE(0, 0)
buffer.writeUInt32LE(ramDiskSize,     4)
buffer.writeUInt32LE(assets.length,   8)
const assetInfoSize = 256 + 4 + 4
let assetInfoOffset = 4 + 4 + 4
let assetDataOffset = ramDiskSize
for (const asset of assets) {
	const assetSize = fs.statSync('assets\\' + asset).size
	buffer.write(asset, assetInfoOffset, "utf-8")
	buffer.writeUInt32LE(assetSize, assetInfoOffset + 256)

	const file = fs.readFileSync('assets\\' + asset)
	assetDataOffset -= assetSize
	file.copy(buffer, assetDataOffset)

	buffer.writeUInt32LE(assetDataOffset, assetInfoOffset + 256 + 4)
	assetInfoOffset += assetInfoSize
}

function getChecksum() {
	let checksum = 0
	for (let i = 1; i < Math.round(ramDiskSize / 4); i++) // Start from 1, coz 0 contains original checksum
		// Well, needs better checksum... JavaScript messes with numbers
		checksum = buffer.readUInt32LE(i * 4) & 0xFFFF ^ checksum
	return checksum
}

let ramDiskChecksum = getChecksum()
buffer.writeUInt32LE(ramDiskChecksum, 0)
console.log('ramDiskChecksum is', ramDiskChecksum)

fs.writeFile("R:\\TOFITA.DAT", buffer, "binary", function(error) {
	if (error) console.error(error)
})
