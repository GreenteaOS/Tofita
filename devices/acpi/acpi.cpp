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

namespace acpi {
	struct ACPI10 {
		char signature[8];
		uint8_t checksum;
		char oemID[6];
		uint8_t revision;
		uint32_t rsdtAddress;
	} __attribute__ ((packed));

	/// ACPI20 contains ACPI10 structure
	struct ACPI20 {
		char signature[8];
		uint8_t checksum;
		char oemID[6];
		uint8_t revision;
		uint32_t rsdtAddress;

		uint32_t length;
		uint64_t xsdtAddress;
		uint8_t checksum2;
		uint8_t reserved[3];
	} __attribute__ ((packed));

	ACPI20* acpi20 = nullptr;

	bool parse(void* acpiVendorTable) {
		serialPrint("[ACPI] parsing started at ");
		serialPrintHex((uint64_t) (acpiVendorTable));
		serialPrint("\n");

		if (acpiVendorTable == nullptr) {
			serialPrintln("[ACPI] not present at all");
			return false;
		}

		const ACPI10* acpi10 = (const ACPI10*)acpiVendorTable;
		serialPrint("[ACPI] signature is '");
		for (int i = 0; i < sizeof(acpi10->signature); i++)
			putchar(acpi10->signature[i]);
		serialPrint("'\n");

		const char signature[9] = "RSD PTR ";

		for (int i = 0; i < sizeof(acpi10->signature); i++)
			if (acpi10->signature[i] != signature[i]) {
				serialPrintln("[ACPI] RSDP table signature is incorrect");
				return false;
			}

		uint8_t checksum = 0;
		for (int i = 0; i < sizeof(ACPI10); i++) checksum += ((uint8_t*)acpi10)[i];
		if (checksum != 0) {
			serialPrintln("[ACPI] checksum ACPI 1.0 failed");
			return false;
		}

		const ACPI20* acpi20 = (const ACPI20*)acpiVendorTable;

		serialPrintf("[ACPI] revision is %d\n", acpi20->revision);

		serialPrintln("[ACPI] loaded");
		return true;
	}

	/// `false` if failed to do so
	/// TODO `poweroff` quake command
	bool shutdownComputer() {
		return false;
	}

	/// `false` if failed to do so
	/// TODO `reboot` quake command
	bool rebootComputer() {
		return false;
	}
}

