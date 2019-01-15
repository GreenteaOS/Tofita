# Assets

Resources bundled into initial RAM disk.

Structure of `TOFITA.DAT`:

```c
uint32_t ramDiskChecksum;
uint32_t ramDiskSize;
uint32_t assetsCount;
struct assetDefinition
{
	uint8_t path[256];
	uint32_t size;
	uint32_t offsetFromRamDiskFirstByte; // NOT relative to `data`
};
uint8_t* data;
```
