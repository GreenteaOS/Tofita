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

// ACPI 2.0 is required by Windows 7
// So we don't need to support ACPI 1.0
// TODO check if more generic table ST->ConfigurationTable viable

namespace acpi {

// Types

constexpr uint32_t byteswap(uint32_t x) {
	return ((x >> 24) & 0x000000ff) | ((x >> 8) & 0x0000ff00) |
		   ((x << 8) & 0x00ff0000) | ((x << 24) & 0xff000000);
}

#define TABLE_HEADER(signature)                                               \
	static const uint32_t typeId = byteswap(signature);                       \
	AcpiTableHeader header;

struct AcpiTableHeader {
	uint32_t type;
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char8_t oem_id[6];
	char8_t oem_table[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;

	bool validate(uint32_t expectedType = 0) const;
} __attribute__((packed));

bool AcpiTableHeader::validate(uint32_t expectedType) const {
	return !expectedType || (expectedType == type);
}

struct AcpiMcfgEntry {
	uint64_t base;
	uint16_t group;
	uint8_t busStart;
	uint8_t busEnd;
	uint32_t reserved;
} __attribute__((packed));

struct AcpiMcfg {
	TABLE_HEADER('MCFG');
	uint64_t reserved;
	AcpiMcfgEntry entries[0];
} __attribute__((packed));

struct AcpiApic {
	TABLE_HEADER('APIC');
	uint32_t localAddress;
	uint32_t flags;
	uint8_t controllerData[0];
} __attribute__((packed));

struct XSDT {
	TABLE_HEADER('XSDT');
	AcpiTableHeader *headers[0];
} __attribute__((packed));

struct Acpi20 {
	uint32_t length;
	Physical<XSDT> xsdtAddress;
	uint8_t checksum;
	uint8_t reserved[3];
} __attribute__((packed));

struct Acpi10 {
	char8_t signature[8];
	uint8_t checksum;
	char8_t oemID[6];
	uint8_t revision;
	uint32_t rsdtAddress;
} __attribute__((packed));

struct ACPI {
	Acpi10 acpi10;
	Acpi20 acpi20;
} __attribute__((packed));

extern "C++" template <typename T>
uint64_t acpiTableEntries(const T *t, uint64_t size) {
	return (t->header.length - sizeof(T)) / size;
}

extern "C++" template <typename T> bool acpiValidate(const T *t) {
	return t->header.validate(T::typeId);
}

static void put_sig(char8_t *into, uint32_t type) {
	for (int32_t j = 0; j < 4; ++j)
		into[j] = reinterpret_cast<char8_t *>(&type)[j];
}

extern "C++" template <typename T> inline T *offset_pointer(T *p, uint64_t n) {
	return reinterpret_cast<T *>(reinterpret_cast<uint64_t>(p) + n);
}

static inline uint16_t busAddr(uint8_t bus, uint8_t device, uint8_t func) {
	return bus << 8 | device << 3 | func;
}

struct PciGroup {
	uint16_t group;
	uint16_t busStart;
	uint16_t busEnd;
	uint32_t *base;

	uint32_t *baseFor(uint8_t bus, uint8_t device, uint8_t func) {
		return offset_pointer(base, busAddr(bus, device, func) << 12);
	}

	bool hasDevice(uint8_t bus, uint8_t device, uint8_t func) {
		return (*baseFor(bus, device, func) & 0xffff) != 0xffff;
	}
};

}

class ACPIParser {
public:
	static bool parse(uint64_t acpiVendorTable) {
		serialPrint(u8"[ACPI] parsing started at physical ");
		serialPrintHex((uint64_t)(acpiVendorTable));
		serialPrint(u8"\n");

		if (acpiVendorTable == 0) {
			serialPrintln(u8"[ACPI] not present at all");
			return false;
		}

		auto acpiTable = (const acpi::ACPI *)physicalToVirtual(acpiVendorTable);

		auto acpi10 = &acpiTable->acpi10;

		serialPrint(u8"[ACPI] signature is '");
		for (int32_t i = 0; i < sizeof(acpi10->signature); i++)
			putchar(acpi10->signature[i]);
		serialPrint(u8"'\n");

		serialPrintf(u8"[ACPI] revision is %d\n", acpi10->revision);

		const char8_t signature[9] = u8"RSD PTR ";

		for (int32_t i = 0; i < sizeof(acpi10->signature); i++)
			if (acpi10->signature[i] != signature[i]) {
				serialPrintln(u8"[ACPI] RSDP table signature is incorrect");
				return false;
			}

		uint8_t checksum = 0;
		for (int32_t i = 0; i < sizeof(acpi::Acpi10); i++)
			checksum += ((uint8_t *)acpi10)[i];
		if (checksum != 0) {
			serialPrintln(u8"[ACPI] checksum ACPI 1.0 failed");
			return false;
		}

		let acpi20data = (const acpi::Acpi20 *)(&acpiTable->acpi20);
		let acpi20raw = (const uint8_t *)acpi20data;
		checksum = 0;
		for (int32_t i = 0; i < sizeof(acpi::Acpi20); i++)
			checksum += (acpi20raw)[i];
		if (checksum != 0) {
			serialPrintln(u8"[ACPI] checksum ACPI 2.0 failed");
			return false;
		}

		auto xsdt = acpiTable->acpi20.xsdtAddress.toVirtual();

		loadXsdt(xsdt);

		serialPrintln(u8"[ACPI] loaded");
		return true;
	}

private:
	static uint64_t physicalToVirtual(uint64_t physical) {
		uint64_t result = (uint64_t)WholePhysicalStart + (uint64_t)physical;
		return result;
	}

	static function dump32(const char8_t *msg, const void *virtualAddress) {
		serialPrintf(u8"\n[DUMP] %s: ", msg);
		const char8_t *x = (char8_t *)((uint64_t)(virtualAddress));
		const uint8_t *y = (uint8_t *)((uint64_t)(virtualAddress));
		for (int32_t j = 0; j < 8; ++j) {
			char8_t buffer[2];
			buffer[0] = (y[j] == 0 || y[j] < 32) ? '?' : x[j];
			buffer[1] = (char8_t)0;
			serialPrint(buffer);
		}
		serialPrint(u8"\n");
	}

	static function loadXsdt(const acpi::XSDT *xsdt) {
		if (xsdt == null)
			return;

		serialPrintf(u8"[ACPI] acpiValidate(xsdt) %d\n", acpiValidate(xsdt));

		char8_t sig[5] = {0, 0, 0, 0, 0};
		serialPrintf(u8"[ACPI] ACPI 2.0+ tables loading\n");
		acpi::put_sig(sig, xsdt->header.type);
		serialPrintf(u8"[ACPI] Found table %s\n", sig);
		quakePrintf(u8"[ACPI] Found table %s, ", sig);

		uint64_t numTables = acpiTableEntries(xsdt, sizeof(void *));
		serialPrintf(u8"[ACPI] acpiTableEntries %d\n", numTables);

		for (uint64_t i = 0; i < numTables; ++i) {
			auto header =
				(const acpi::AcpiTableHeader *)physicalToVirtual((uint64_t)xsdt->headers[i]);

			acpi::put_sig(sig, header->type);
			serialPrintf(u8"[ACPI] Found table %s\n", sig);
			quakePrintf(u8"found table %s, ", sig);

			serialPrintf(u8"[ACPI] acpiValidate(header) %d\n", header->validate());

			switch (header->type) {
			case acpi::AcpiApic::typeId:
				loadApic((const acpi::AcpiApic *)(header));
				break;

			case acpi::AcpiMcfg::typeId:
				loadMcfg((const acpi::AcpiMcfg *)(header));
				break;

			default:
				break;
			}
		}

		quakePrintf(u8"done.\n");
	}

	static function loadApic(const acpi::AcpiApic *apic) {
		serialPrintf(u8"loadApic\n");
		uint32_t *local = reinterpret_cast<uint32_t *>(apic->localAddress);
	}

	static function loadMcfg(const acpi::AcpiMcfg *mcfg) {
		serialPrintf(u8"loadMcfg\n");
		uint64_t count = acpiTableEntries(mcfg, sizeof(acpi::AcpiMcfgEntry));
		acpi::PciGroup mPci[count];

		for (uint32_t i = 0; i < count; ++i) {
			const acpi::AcpiMcfgEntry &mcfge = mcfg->entries[i];

			mPci[i].group = mcfge.group;
			mPci[i].busStart = mcfge.busStart;
			mPci[i].busEnd = mcfge.busEnd;
			mPci[i].base = reinterpret_cast<uint32_t *>(mcfge.base);

			int32_t num_busses = mPci[i].busEnd - mPci[i].busStart + 1;
		}

		probePci(mPci, count);
	}

	static bool multi(acpi::PciGroup *group, uint8_t bus, uint8_t device, uint8_t func) {
		uint32_t *mBase = group->baseFor(bus, device, func);
		auto mMulti = ((mBase[3] >> 16) & 0x80) == 0x80;
		return mMulti;
	}

	static function probePci(acpi::PciGroup *mPci, uint64_t count) {
		serialPrintf(u8"[PCI] probePci\n");
		for (uint32_t i = 0; i < count; ++i) {
			acpi::PciGroup* pci = &mPci[i];
			for (int32_t bus = pci->busStart; bus <= pci->busEnd; ++bus) {
				for (int32_t dev = 0; dev < 32; ++dev) {
					// TODO
				}
			}
		}
	}
};

// Management

/// `false` if failed to do so
/// TODO `poweroff` quake command
bool shutdownComputer() { return false; }

/// `false` if failed to do so
/// TODO `reboot` quake command
bool rebootComputer() { return false; }
