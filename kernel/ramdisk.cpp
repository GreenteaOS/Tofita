// The Tofita Kernel
// Copyright (C) 2020  Oleg Petrenko
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
	uint32_t ramDiskChecksum;
	uint32_t ramDiskSize;
	uint32_t assetsCount;
} RamDiskInfo;

typedef struct {
	uint16_t path[128];
	uint32_t size;
	uint32_t offsetFromRamDiskFirstByte;
} RamDiskAssetInfo;

static const RamDisk *_ramdisk;
function setRamDisk(const RamDisk *ramdisk) {
	_ramdisk = ramdisk;
}

typedef struct {
	uint32_t size;
	uint8_t *data;
} RamDiskAsset;

const RamDiskAsset getRamDiskAsset(const wchar_t *pathToAsset) {
	RamDiskAsset asset;
	asset.size = 0;
	asset.data = 0;
	const uint16_t *path = (const uint16_t *)pathToAsset;

	const RamDiskInfo *ramDiskInfo = (const RamDiskInfo *)_ramdisk->base;

	for (uint32_t id = 0; id < ramDiskInfo->assetsCount; id++) {
		// Pointer arithmetic like a boss
		const uint64_t ramDiskAssetInfoPtr =
			(uint64_t)_ramdisk->base + sizeof(RamDiskInfo) + sizeof(RamDiskAssetInfo) * id;
		const RamDiskAssetInfo *ramDiskAssetInfo = (const RamDiskAssetInfo *)ramDiskAssetInfoPtr;
		uint8_t found = 1;

		for (uint8_t at = 0; at < 127; at++) {
			if (ramDiskAssetInfo->path[at] != path[at]) {
				found = 0;
				break;
			}
			if (ramDiskAssetInfo->path[at] == 0)
				break;
		}

		if (found) {
			asset.size = ramDiskAssetInfo->size;
			asset.data = (uint8_t *)((uint64_t)_ramdisk->base + ramDiskAssetInfo->offsetFromRamDiskFirstByte);
			return asset;
		}
	}

	serialPrintf(L"[ramdisk.getRamDiskAsset] asset '%S' not found in %d assets\n", path,
				 ramDiskInfo->assetsCount);
	return asset;
}
