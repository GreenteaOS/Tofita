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

efi::EFI_STATUS loadRamDiskFromVolume(efi::EFI_BOOT_SERVICES *bootsvc, efi::EFI_FILE_PROTOCOL *root, RamDisk* ramdisk)
{
	efi::EFI_STATUS status;
	static efi::CHAR16 *name = (efi::CHAR16 *)L"TOFITA.DAT";

	efi::EFI_FILE_PROTOCOL *file = NULL;
	status = root->Open(root, &file, (efi::CHAR16 *)name, EFI_FILE_MODE_READ,
						EFI_FILE_READ_ONLY | EFI_FILE_HIDDEN | EFI_FILE_SYSTEM);

	if (status == EFI_NOT_FOUND)
		return status;

	serialPrintf("[[[efi_main.loadRamDiskFromVolume]]] status: Open %d\n", status);

	char info[sizeof(efi::EFI_FILE_INFO) + 100];
	size_t infoSize = sizeof(info);

	efi::EFI_GUID GenericFileInfo = EFI_FILE_INFO_ID;

	status = file->GetInfo(file, &GenericFileInfo, &infoSize, info);
	serialPrintf("[[[efi_main.loadRamDiskFromVolume]]] status: GetInfo %d\n", status);

	size_t size = ((efi::EFI_FILE_INFO *)info)->FileSize;
	serialPrintf("[[[efi_main.loadRamDiskFromVolume]]] FileSize %d\n", size);

	void *address = (void*)0;
	status = uefiAllocate(
			bootsvc,
			&size,
			&address);
	serialPrintf("[[[efi_main.loadRamDiskFromVolume]]] status: uefiAllocate %d, size %d at %d\n", status, size, address);

	status = file->Read(file, &size, address);
	serialPrintf("[[[efi_main.loadRamDiskFromVolume]]] status: Read %d\n", status);

	status = file->Close(file);
	serialPrintf("[[[efi_main.loadRamDiskFromVolume]]] status: Close %d\n", status);

	uint32_t* uints = (uint32_t*)(address);
	uint32_t checksize = size / 4; // `size` is rounded upto 4 bytes by assets generator
	uint32_t checksum = 0;

	for (uint32_t i = 1; i < checksize; i++) // Start from 1, coz 0 contains original checksum
		checksum = uints[i] & 0xFFFF ^ checksum;

	serialPrintf("[[[efi_main.loadRamDiskFromVolume]]] checksum: %d expected: %d\n", checksum, uints[0]);
	if (checksum != uints[0])
		serialPrintln("[[[efi_main.loadRamDiskFromVolume]]] <ERROR> checksum is incorrect, disk may be corrupted!");

	ramdisk->base = (uint64_t)address;
	ramdisk->size = size;

	return EFI_SUCCESS;
}

// returns EFI_SUCCESS or EFI_NOT_FOUND
efi::EFI_STATUS findAndLoadRamDisk(efi::EFI_BOOT_SERVICES *bootsvc, RamDisk* ramdisk) {
	efi::EFI_STATUS status = EFI_NOT_READY;
	efi::EFI_HANDLE *handleBuffer = NULL;
	size_t handleCount = 0;

	efi::EFI_GUID simpleFileSystemProtocol = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

	status = bootsvc->LocateHandleBuffer(efi::ByProtocol, &simpleFileSystemProtocol, NULL, &handleCount, &handleBuffer);
	if (status != EFI_SUCCESS) serialPrintf("[[[efi_main.findAndLoadRamDisk]]] <ERROR> failed: LocateHandleBuffer %d\n", status);
	else serialPrintf("[[[efi_main.findAndLoadRamDisk]]] success: LocateHandleBuffer, got %d handles\n", handleCount);

	for (size_t i = 0; i < handleCount; ++i) {
		serialPrintf("[[[efi_main.findAndLoadRamDisk]]] loading handle #%d of %d handles\n", i, handleCount);
		efi::EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fileSystem = NULL;

		status = bootsvc->HandleProtocol(handleBuffer[i], &simpleFileSystemProtocol, (void **)&fileSystem);

		if (status != EFI_SUCCESS) {
			serialPrintf("[[[efi_main.findAndLoadRamDisk]]] HandleProtocol got a non-file system (status %d, handle %d), continue\n", status, i);
			continue ;
		}

		serialPrintln("[[[efi_main.findAndLoadRamDisk]]] success: HandleProtocol found a file system");
		efi::EFI_FILE_PROTOCOL *root = NULL;
		status = fileSystem->OpenVolume(fileSystem, &root);
		if (status != EFI_SUCCESS) {
			serialPrintf("[[[efi_main.findAndLoadRamDisk]]] failed: OpenVolume with status %d, continue to the next one\n", status);
			continue ;
		}

		status = loadRamDiskFromVolume(bootsvc, root, ramdisk);
		if (status != EFI_SUCCESS) {
			serialPrintf("[[[efi_main.findAndLoadRamDisk]]] failed: loadRamDiskFromVolume with status %d, continue to the next one\n", status);
			continue ;
		}

		return EFI_SUCCESS;
	}

	serialPrintf("[[[efi_main.findAndLoadRamDisk]]] <ERROR> failed to find RAM disk at all\n");
	return EFI_NOT_FOUND;
}
