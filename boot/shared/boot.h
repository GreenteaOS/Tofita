#define MEMORY_MAP_BUFFER_SIZE 512 * 1024 // 512 KiB
uint8_t memoryMapBuffer[MEMORY_MAP_BUFFER_SIZE];

typedef struct {
	EFI_MEMORY_DESCRIPTOR *memoryMap;
	uint64_t memoryMapSize;
	uint64_t mapKey;
	uint64_t descriptorSize;
	uint32_t descriptorVersion;
} EfiMemoryMap;

typedef struct {
	void *base; // physical address
	uint32_t size; // in bytes
	uint16_t width;
	uint16_t height;
} Framebuffer;

// Start of kernel sections in memory, see loader.ld
#define KERNEL_START (1 * 1024 * 1024)

#define KERNEL_VIRTUAL_BASE 0xffff800000000000
#define EFI_VIRTUAL_BASE (KERNEL_VIRTUAL_BASE + 0x40000000)
#define FRAMEBUFFER_START (EFI_VIRTUAL_BASE + 0x40000000)

typedef struct {
	const EFI_HANDLE imageHandle;
	EfiMemoryMap efiMemoryMap;
	EFI_RUNTIME_SERVICES *efiRuntimeServices;
	Framebuffer framebuffer;
} KernelParams;

typedef void (*InitKernel)(const KernelParams *);
