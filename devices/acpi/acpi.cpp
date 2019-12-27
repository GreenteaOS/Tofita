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

#define TABLE_HEADER(signature)                                                \
	static const uint32_t type_id = byteswap(signature);                       \
	acpi_table_header header;

struct acpi_table_header {
	uint32_t type;
	uint32_t length;
	uint8_t revision;
	uint8_t checksum;
	char8_t oem_id[6];
	char8_t oem_table[8];
	uint32_t oem_revision;
	uint32_t creator_id;
	uint32_t creator_revision;

	bool validate(uint32_t expected_type = 0) const;
} __attribute__((packed));

bool acpi_table_header::validate(uint32_t expected_type) const {
	return !expected_type || (expected_type == type);
}

struct acpi_mcfg_entry {
	uint64_t base;
	uint16_t group;
	uint8_t bus_start;
	uint8_t bus_end;
	uint32_t reserved;
} __attribute__((packed));

struct acpi_mcfg {
	TABLE_HEADER('MCFG');
	uint64_t reserved;
	acpi_mcfg_entry entries[0];
} __attribute__((packed));

struct acpi_apic {
	TABLE_HEADER('APIC');
	uint32_t local_address;
	uint32_t flags;
	uint8_t controller_data[0];
} __attribute__((packed));

struct XSDT {
	TABLE_HEADER('XSDT');
	acpi_table_header *headers[0];
} __attribute__((packed));

struct ACPI20 {
	uint32_t length;
	XSDT *xsdtAddress;
	uint8_t checksum;
	uint8_t reserved[3];
} __attribute__((packed));

struct ACPI10 {
	char8_t signature[8];
	uint8_t checksum;
	char8_t oemID[6];
	uint8_t revision;
	uint32_t rsdtAddress;
} __attribute__((packed));

struct ACPI {
	ACPI10 acpi10;
	ACPI20 acpi20;
} __attribute__((packed));

extern "C++" template <typename T>
uint64_t acpi_table_entries(const T *t, uint64_t size) {
	return (t->header.length - sizeof(T)) / size;
}

extern "C++" template <typename T> bool acpi_validate(const T *t) {
	return t->header.validate(T::type_id);
}

static void put_sig(char8_t *into, uint32_t type) {
	for (int32_t j = 0; j < 4; ++j)
		into[j] = reinterpret_cast<char8_t *>(&type)[j];
}
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
		for (int32_t i = 0; i < sizeof(acpi::ACPI10); i++)
			checksum += ((uint8_t *)acpi10)[i];
		if (checksum != 0) {
			serialPrintln(u8"[ACPI] checksum ACPI 1.0 failed");
			return false;
		}

		let acpi20data = (const acpi::ACPI20 *)(&acpiTable->acpi20);
		let acpi20raw = (const uint8_t *)acpi20data;
		checksum = 0;
		for (int32_t i = 0; i < sizeof(acpi::ACPI20); i++)
			checksum += (acpi20raw)[i];
		if (checksum != 0) {
			serialPrintln(u8"[ACPI] checksum ACPI 2.0 failed");
			return false;
		}

		auto xsdt = (const acpi::XSDT *)physicalToVirtual((uint64_t)acpiTable->acpi20.xsdtAddress);

		loadXsdt(xsdt);

		serialPrintln(u8"[ACPI] loaded");
		return true;
	}

private:
	static uint64_t physicalToVirtual(uint64_t physical) {
		uint64_t result = (uint64_t)WholePhysicalStart + (uint64_t)physical;
		return result;
	}

	static void dump32(const char8_t *msg, const void *virtualAddress) {
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

		serialPrintf(u8"[ACPI] acpi_validate(xsdt) %d\n", acpi_validate(xsdt));

		char8_t sig[5] = {0, 0, 0, 0, 0};
		serialPrintf(u8"[ACPI] ACPI 2.0+ tables loading\n");
		acpi::put_sig(sig, xsdt->header.type);
		serialPrintf(u8"[ACPI] Found table %s\n", sig);
		quakePrintf(u8"[ACPI] Found table %s, ", sig);

		uint64_t num_tables = acpi_table_entries(xsdt, sizeof(void *));
		serialPrintf(u8"[ACPI] acpi_table_entries %d\n", num_tables);

		for (uint64_t i = 0; i < num_tables; ++i) {
			auto header =
				(const acpi::acpi_table_header *)physicalToVirtual((uint64_t)xsdt->headers[i]);

			acpi::put_sig(sig, header->type);
			serialPrintf(u8"[ACPI] Found table %s\n", sig);
			quakePrintf(u8"found table %s, ", sig);

			serialPrintf(u8"[ACPI] acpi_validate(header) %d\n", header->validate());

			switch (header->type) {
			case acpi::acpi_apic::type_id:
				break;

			case acpi::acpi_mcfg::type_id:
				break;

			default:
				break;
			}
		}

		quakePrintf(u8"done.\n");
	}
	}

	}


// Management

/// `false` if failed to do so
/// TODO `poweroff` quake command
bool shutdownComputer() { return false; }

/// `false` if failed to do so
/// TODO `reboot` quake command
bool rebootComputer() { return false; }
