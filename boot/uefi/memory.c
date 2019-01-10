void fillMemoryMap(EfiMemoryMap *efiMemoryMap) {
	efiMemoryMap->memoryMap = (EFI_MEMORY_DESCRIPTOR *) memoryMapBuffer;
	efiMemoryMap->memoryMapSize = MEMORY_MAP_BUFFER_SIZE;

	EFI_STATUS status = ST->BootServices->GetMemoryMap(
		&efiMemoryMap->memoryMapSize,
		efiMemoryMap->memoryMap,
		&efiMemoryMap->mapKey,
		&efiMemoryMap->descriptorSize,
		&efiMemoryMap->descriptorVersion);

	if (status != EFI_SUCCESS) {
		serialPrint("[[[efi_main.fillMemoryMap]]] <ERROR> GetMemoryMap: failed\r\n");
	}
}

void initializeFramebuffer(Framebuffer *fb) {
	EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;

	ST->BootServices->LocateProtocol(&gopGuid, NULL, (void **) &gop);

	fb->base = (void *) gop->Mode->FrameBufferBase;
	fb->size = gop->Mode->FrameBufferSize;

	fb->width = gop->Mode->Info->HorizontalResolution;
	fb->height = gop->Mode->Info->VerticalResolution;

	gop->SetMode(gop, gop->Mode->Mode);
}

EFI_STATUS uefiAllocate(EFI_BOOT_SERVICES *bootsvc, EFI_MEMORY_TYPE allocationType, size_t *bytes, void **destination)
{
	EFI_STATUS status;

	// Round to page size
	size_t pages = ((*bytes - 1) / PAGE_SIZE) + 1;
	EFI_PHYSICAL_ADDRESS addr = (EFI_PHYSICAL_ADDRESS)*destination;

	status = bootsvc->AllocatePages(AllocateAddress, allocationType, pages, &addr);
	if (status == EFI_NOT_FOUND || status == EFI_OUT_OF_RESOURCES) {
		serialPrintf("[[[efi_main.uefiAllocate]]] failed: AllocateAddress status %d, using AllocateAnyPages\r\n", status);
		status = bootsvc->AllocatePages(AllocateAnyPages, allocationType, pages, &addr);
	}

	if (status != EFI_SUCCESS)
		serialPrintf("[[[efi_main.uefiAllocate]]] failed: AllocateAnyPages %d bytes, rounded to %d pages, of type %d, status %d\r\n", bytes, pages, allocationType, status);

	*bytes = pages * PAGE_SIZE;
	*destination = (void *)addr;

	return status;
}
