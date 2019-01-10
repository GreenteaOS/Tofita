// returns EFI_SUCCESS or EFI_NOT_FOUND
EFI_STATUS findAndLoadRamDisk(EFI_BOOT_SERVICES *bootsvc) {
	EFI_STATUS status = EFI_NOT_READY;
	EFI_HANDLE *handleBuffer = NULL;
	size_t handleCount = 0;

	EFI_GUID simpleFileSystemProtocol = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;

	status = bootsvc->LocateHandleBuffer(ByProtocol, &simpleFileSystemProtocol, NULL, &handleCount, &handleBuffer);
	if (EFI_ERROR(status)) serialPrintf("[[[efi_main.findAndLoadRamDisk]]] <ERROR> failed: LocateHandleBuffer %d\r\n", status);
	else serialPrintf("[[[efi_main.findAndLoadRamDisk]]] success: LocateHandleBuffer, got %d handleBuffer\r\n", handleCount);

	for (size_t i = 0; i < handleCount; ++i) {
		serialPrintf("[[[efi_main.findAndLoadRamDisk]]] loading handle #%d from %ds\r\n", i, handleCount);
		EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *fileSystem = NULL;

		status = bootsvc->HandleProtocol(handleBuffer[i], &simpleFileSystemProtocol, (void **)&fileSystem);

		if (EFI_ERROR(status)) {
			serialPrintf("[[[efi_main.findAndLoadRamDisk]]] HandleProtocol got a non-file system (status %d, handle %d), continue\r\n", status, i);
		} else {
			serialPrintln("[[[efi_main.findAndLoadRamDisk]]] success: HandleProtocol found a file system");
			EFI_FILE_PROTOCOL *root = NULL;
			status = fileSystem->OpenVolume(fileSystem, &root);
		}
	}
}
