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

typedef struct {
	uint32_t ramDiskSize;
	uint32_t assetsCount;
} RamDiskInfo;

typedef struct {
	uint8_t path[256];
	uint32_t size;
	uint32_t offsetFromRamDiskFirstByte;
} RamDiskAssetInfo;

static RamDisk *_ramdisk;
void setRamDisk(RamDisk *ramdisk) {
	_ramdisk = ramdisk;
}

typedef struct {
	uint32_t size;
	uint8_t *data;
} RamDiskAsset;

RamDiskAsset getRamDiskAsset(uint8_t *path) {
	RamDiskAsset asset;
	asset.size = 0;
	asset.data = 0;

	RamDiskInfo* ramDiskInfo = _ramdisk->base;

	for (uint32_t id = 0; id < ramDiskInfo->assetsCount; id++) {
		// Pointer arithmetic like a boss
		RamDiskAssetInfo* ramDiskAssetInfo = _ramdisk->base + sizeof(RamDiskInfo) + sizeof(RamDiskAssetInfo) * id;
		uint8_t found = 1;

		for (uint8_t at = 0; at < 255; at++) {
			if (ramDiskAssetInfo->path[at] != path[at]) {
				found = 0;
				break;
			}
			if (ramDiskAssetInfo->path[at] == 0) break ;
		}

		if (found) {
			asset.size = ramDiskAssetInfo->size;
			asset.data = _ramdisk->base + ramDiskAssetInfo->offsetFromRamDiskFirstByte;
			return asset;
		}
	}

	serialPrintf("[ramdisk.getRamDiskAsset] asset '%s' not found in %d assets\r\n", path, ramDiskInfo->assetsCount);
	return asset;
}
