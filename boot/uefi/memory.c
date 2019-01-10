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

