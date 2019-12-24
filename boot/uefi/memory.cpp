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

function fillMemoryMap(EfiMemoryMap *efiMemoryMap, efi::EFI_SYSTEM_TABLE *systemTable) {
	efi::EFI_STATUS status = systemTable->BootServices->GetMemoryMap(
		&efiMemoryMap->memoryMapSize,
		efiMemoryMap->memoryMap,
		&efiMemoryMap->mapKey,
		&efiMemoryMap->descriptorSize,
		&efiMemoryMap->descriptorVersion);

	if (status != EFI_SUCCESS) {
		serialPrint("[[[efi_main.fillMemoryMap]]] <ERROR> GetMemoryMap: failed\n");
	}

	serialPrintf("[[[efi_main.fillMemoryMap]]] memoryMapSize %d, descriptorSize %d\n",
		efiMemoryMap->memoryMapSize,
		efiMemoryMap->descriptorSize
	);
}

function initializeFramebuffer(Framebuffer *fb, efi::EFI_SYSTEM_TABLE *systemTable) {
	efi::EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	efi::EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

	systemTable->BootServices->LocateProtocol(&gopGuid, NULL, (void **) &gop);

	fb->base = gop->Mode->FrameBufferBase;
	fb->size = gop->Mode->FrameBufferSize;

	fb->width = gop->Mode->Info->HorizontalResolution;
	fb->height = gop->Mode->Info->VerticalResolution;

	gop->SetMode(gop, gop->Mode->Mode);
}

efi::EFI_STATUS uefiAllocate(efi::EFI_BOOT_SERVICES *bootsvc, size_t *bytes, void **destination)
{
	serialPrintf("[[[efi_main.uefiAllocate]]] start allocating %d bytes\n", *bytes);
	efi::EFI_STATUS status;
	// HINT: Data in EfiRuntimeServicesData will be preserved when exiting bootservices and always available
	efi::EFI_MEMORY_TYPE allocationType = efi::EfiLoaderCode; // Use *Code not *Data to avoid NX-bit crash if data executed

	// Round to page size
	size_t pages = ((*bytes - 1) / PAGE_SIZE) + 1;
	efi::EFI_PHYSICAL_ADDRESS addr = (efi::EFI_PHYSICAL_ADDRESS)*destination;

	status = bootsvc->AllocatePages(efi::AllocateAnyPages, allocationType, pages, &addr);
	if (status == EFI_NOT_FOUND || status == EFI_OUT_OF_RESOURCES) {
		serialPrintf("[[[efi_main.uefiAllocate]]] failed: EFI_NOT_FOUND/EFI_OUT_OF_RESOURCES for %d bytes\n", *bytes);
	}

	if (status != EFI_SUCCESS)
		serialPrintf("[[[efi_main.uefiAllocate]]] failed: AllocateAnyPages %d bytes, %d pages, of type %d, status %d\n", *bytes, pages, allocationType, status);

	*bytes = pages * PAGE_SIZE;
	*destination = (void *)addr;

	return status;
}
