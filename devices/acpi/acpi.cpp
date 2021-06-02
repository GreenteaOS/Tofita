// The Tofita Kernel
// Copyright (C) 2020  Oleg Petrenko
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

// ACPI 2.0 is required by Tofita
// So we don't need to support ACPI 1.0
// TODO check if more generic table ST->ConfigurationTable viable

namespace acpi {

// Types

constexpr uint32_t byteswap(uint32_t x) {
	return ((x >> 24) & 0x000000ff) | ((x >> 8) & 0x0000ff00) | ((x << 8) & 0x00ff0000) |
		   ((x << 24) & 0xff000000);
}

#define TABLE_HEADER(signature)                                                                              \
	static const uint32_t typeId = byteswap(signature);                                                      \
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

SIZEOF(ACPI, sizeof(Acpi10) + sizeof(Acpi20))

struct ApicHeader {
	uint8_t type;
	uint8_t length;
} __attribute__((packed));

struct ApicIoApic {
	ApicHeader header;
	uint8_t ioApicId;
	uint8_t reserved;
	uint32_t ioApicAddress;
	uint32_t globalSystemInterruptBase;
} __attribute__((packed));

struct ApicInterruptOverride {
	ApicHeader header;
	uint8_t bus;
	uint8_t source;
	uint32_t interrupt;
	uint16_t flags;
} __attribute__((packed));

extern "C++" template <typename T> uint64_t acpiTableEntries(const T *t, uint64_t size) {
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

} // namespace acpi

class ACPIParser {
  public:
	static bool parse(uint64_t acpiVendorTable) {
		serialPrint(L"[ACPI] parsing started at physical ");
		serialPrintHex((uint64_t)(acpiVendorTable));
		serialPrint(L"\n");

		if (acpiVendorTable == 0) {
			serialPrintln(L"[ACPI] not present at all");
			return false;
		}

		auto acpiTable = (const acpi::ACPI *)physicalToVirtual(acpiVendorTable);

		auto acpi10 = &acpiTable->acpi10;

		serialPrint(L"[ACPI] signature is '");
		for (int32_t i = 0; i < sizeof(acpi10->signature); i++)
			putchar(acpi10->signature[i]);
		serialPrint(L"'\n");

		serialPrintf(L"[ACPI] revision is %d\n", acpi10->revision);

		const char8_t signature[9] = u8"RSD PTR ";

		for (int32_t i = 0; i < sizeof(acpi10->signature); i++)
			if (acpi10->signature[i] != signature[i]) {
				serialPrintln(L"[ACPI] RSDP table signature is incorrect");
				return false;
			}

		uint8_t checksum = 0;
		for (int32_t i = 0; i < sizeof(acpi::Acpi10); i++)
			checksum += ((uint8_t *)acpi10)[i];
		if (checksum != 0) {
			serialPrintln(L"[ACPI] checksum ACPI 1.0 failed");
			return false;
		}

		let acpi20data = (const acpi::Acpi20 *)(&acpiTable->acpi20);
		let acpi20raw = (const uint8_t *)acpi20data;
		checksum = 0;
		for (int32_t i = 0; i < sizeof(acpi::Acpi20); i++)
			checksum += (acpi20raw)[i];
		if (checksum != 0) {
			serialPrintln(L"[ACPI] checksum ACPI 2.0 failed");
			return false;
		}

		auto xsdt = acpiTable->acpi20.xsdtAddress.toVirtual();

		loadXsdt(xsdt);

		serialPrintln(L"[ACPI] loaded");
		return true;
	}

  private:
	static uint64_t physicalToVirtual(uint64_t physical) {
		uint64_t result = (uint64_t)WholePhysicalStart + (uint64_t)physical;
		return result;
	}

	static function dump32(const char8_t *msg, const void *virtualAddress) {
		serialPrintf(L"\n[DUMP] %s: ", msg);
		const char8_t *x = (char8_t *)((uint64_t)(virtualAddress));
		const uint8_t *y = (uint8_t *)((uint64_t)(virtualAddress));
		for (int32_t j = 0; j < 8; ++j) {
			wchar_t buffer[2];
			buffer[0] = (y[j] == 0 || y[j] < 32) ? '?' : x[j];
			buffer[1] = (char8_t)0;
			serialPrint(buffer);
		}
		serialPrint(L"\n");
	}

	static function loadXsdt(const acpi::XSDT *xsdt) {
		if (xsdt == null)
			return;

		serialPrintf(L"[ACPI] acpiValidate(xsdt) %d\n", acpiValidate(xsdt));

		char8_t sig[5] = {0, 0, 0, 0, 0};
		serialPrintf(L"[ACPI] ACPI 2.0+ tables loading\n");
		acpi::put_sig(sig, xsdt->header.type);
		serialPrintf(L"[ACPI] Found table %s\n", sig);
		quakePrintf(L"[ACPI] Found table %s, ", sig);

		uint64_t numTables = acpiTableEntries(xsdt, sizeof(void *));
		serialPrintf(L"[ACPI] acpiTableEntries %d\n", numTables);

		for (uint64_t i = 0; i < numTables; ++i) {
			auto header = (const acpi::AcpiTableHeader *)physicalToVirtual((uint64_t)xsdt->headers[i]);

			acpi::put_sig(sig, header->type);
			serialPrintf(L"[ACPI] Found table %s\n", sig);
			quakePrintf(L"found table %s, ", sig);

			serialPrintf(L"[ACPI] acpiValidate(header) %d\n", header->validate());

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

		quakePrintf(L"done.\n");
	}

	static volatile uint64_t coresAP;
	static Spinlock apLock;

	static function apStart() {
		// TODO seems useless, cause main CPU waits for SIPIs anyway
		apLock.lock();

		// TODO IDT
		// TODO GDT
		let core = coresAP + 1;
		quakePrintf(L"CPU #%u initialized, ", core);
		amd64::disableAllInterrupts();
		coresAP++;
		apLock.unlock();

		while (true) {
			amd64::halt();
			amd64::pause();
		};
	}

	static function loadApic(const acpi::AcpiApic *apic) {
		amd64::disableAllInterrupts();

		uint32_t APIC_BASE_FLAG = 0xFFFFF000;
		uint32_t APIC_ENABLE_FLAG = 0x800;

		amd64::wrmsr(amd64::MSR::IA32_APIC_BASE, (apic->localAddress & APIC_BASE_FLAG) | APIC_ENABLE_FLAG);

		// +32 GB
		uint64_t virtualLapic = (uint64_t)WholePhysicalStart + ((uint64_t)32 * (uint64_t)1024 * (uint64_t)1024 * (uint64_t)1024);
		uint64_t physicalLapic = apic->localAddress;
		virtualLapicBase = virtualLapic;

		auto localApicOut = [&](uint64_t reg, uint32_t data) {
			auto at = reinterpret_cast<volatile uint32_t *>(virtualLapic + reg);
			at[0] = data;
		};

		auto localApicIn = [&](uint64_t reg) {
			 return amd64::readFrom<volatile uint32_t>(virtualLapic + reg);
		};

		#define LAPIC_TPR 0x0080  // Task Priority
		#define LAPIC_DFR 0x00e0  // Destination Format
		#define LAPIC_LDR 0x00d0  // Logical Destination
		#define LAPIC_SVR 0x00f0  // Spurious Interrupt Vector

		pages::mapMemory(pages::pml4entries, virtualLapic, physicalLapic, 1);

		// Clear task priority to enable all interrupts
		localApicOut(LAPIC_TPR, 0);

		// Logical Destination Mode
		localApicOut(LAPIC_DFR, 0xffffffff);   // Flat mode
		localApicOut(LAPIC_LDR, 0x01000000);   // All cpus use logical id 1

		// Configure Spurious Interrupt Vector Register
		localApicOut(LAPIC_SVR, 0x100 | 0xff);

		// IO APIC

		uint64_t virtualIOapic = (uint64_t)WholePhysicalStart + ((uint64_t)33 * (uint64_t)1024 * (uint64_t)1024 * (uint64_t)1024);
		uint64_t physicalIOapic = 0;

		let count = acpiTableEntries(apic, 1);
		uint8_t const *data = apic->controllerData;
		uint8_t const *end = data + count;

		while (data < end) {
			const uint8_t type = data[0];
			const uint8_t length = data[1];

			// APIC structure types
			#define APIC_TYPE_LOCAL_APIC            0
			#define APIC_TYPE_IO_APIC               1
			#define APIC_TYPE_INTERRUPT_OVERRIDE    2

			switch (type) {
				case APIC_TYPE_IO_APIC: {
					auto s = (acpi::ApicIoApic *)data;

					physicalIOapic = s->ioApicAddress;
				};
				break;
			}

			data += length;
		}

		// TODO 1 page?
		pages::mapMemory(pages::pml4entries, virtualIOapic, physicalIOapic, 1);

		// Memory mapped registers for IO APIC register access
		#define IOREGSEL                        0x00
		#define IOWIN                           0x10

		// IO APIC Registers
		#define IOAPICID                        0x00
		#define IOAPICVER                       0x01
		#define IOAPICARB                       0x02
		#define IOREDTBL                        0x10

		auto ioApicOut = [&](uint64_t base, uint8_t reg, uint32_t val)
		{
			amd64::writeTo<volatile uint32_t>(base + IOREGSEL, reg);
			amd64::writeTo<volatile uint32_t>(base + IOWIN, val);
		};

		auto ioApicIn = [&](uint64_t base, uint8_t reg)
		{
			amd64::writeTo<volatile uint32_t>(base + IOREGSEL, reg);
			return amd64::readFrom<volatile uint32_t>(base + IOWIN);
		};

		auto ioApicSetEntry = [&](uint64_t base, uint8_t index, uint64_t data)
		{
			ioApicOut(base, IOREDTBL + index * 2, (uint32_t)data);
			ioApicOut(base, IOREDTBL + index * 2 + 1, (uint32_t)(data >> 32));
		};

		auto ioApicInit = [&]()
		{
			// Get number of entries supported by the IO APIC
			uint32_t x = ioApicIn(virtualIOapic, IOAPICVER);
			uint32_t count = ((x >> 16) & 0xff) + 1;    // maximum redirection entry

			// Disable all entries
			for (uint32_t i = 0; i < count; ++i)
			{
				ioApicSetEntry(virtualIOapic, i, 1 << 16);
			}
		};

		ioApicInit();

		auto acpiRemapIrq = [&](uint32_t irq) {
			let count = acpiTableEntries(apic, 1);
			uint8_t const *data = apic->controllerData;
			uint8_t const *end = data + count;

			while (data < end) {
				const uint8_t type = data[0];
				const uint8_t length = data[1];

				// APIC structure types
				#define APIC_TYPE_LOCAL_APIC            0
				#define APIC_TYPE_IO_APIC               1
				#define APIC_TYPE_INTERRUPT_OVERRIDE    2

				switch (type) {
					case APIC_TYPE_INTERRUPT_OVERRIDE: {
						auto s = (acpi::ApicInterruptOverride *)data;
						if (s->source == irq) {
							return s->interrupt;
						}
					};
					break;
				}

				data += length;
			}

			return irq;
		};

		// Enable IO APIC entries
		#define INT_TIMER 0x20
		#define IRQ_TIMER 0x00
		ioApicSetEntry(virtualIOapic, acpiRemapIrq(IRQ_TIMER), INT_TIMER);

		localApicOut(LAPIC_EOI, 0);

		// SMP
		// We will fill trapeze arguments dynamically by updating it's binary
		uint64_t trapeze = 0x8000 + (uint64_t)WholePhysicalStart;
		let trapezePhysical = trapeze - (uint64_t)WholePhysicalStart;
		pages::mapMemory(pages::pml4entries, trapeze, trapezePhysical, 1);
		pages::mapMemory(pages::pml4entries, 0x8000 - 4096, 0x8000 - 4096, 16);
		// TODO unmap later

		auto ready = trapeze + 8;
		auto cpu_id = ready + 8;
		auto page_table = cpu_id + 8;

		auto stack_start = page_table + 8;
		auto stack_end = stack_start + 8;
		auto code = stack_end + 8;

		auto pml4 = code + 8;

		data = apic->controllerData;
		let _count = acpiTableEntries(apic, 1);
		end = data + _count;
		uint8_t bsp = 0; // TODO (get from lapic MSR?)

		bool x2 = false; // TODO

		while (data < end) {

			uint32_t volatile * local = reinterpret_cast<uint32_t *>(0);
			const uint8_t type = data[0];
			const uint8_t length = data[1];

			switch (type) {
				case 0:
					uint64_t position = (uint64_t)data;
					uint8_t cpuid = amd64::readFrom<uint8_t>(position + 2);
					uint8_t lapicid = amd64::readFrom<uint8_t>(position + 3);
					uint8_t flags = amd64::readFrom<uint8_t>(position + 4);

					if (bsp == cpuid) {
						break;
					}

					if ((flags & 0x1) == 0) {
						break;
					}

					// Allocate per-core stack
					// TODO decide on stack size (global var)
					let _stack_start = PhysicalAllocator::allocatePages(64);
					let _stack_end = stack_start + 64 * 4096;

					// TODO ensure PML4 & GDT within 4G or even 2G (uint32_t)

					uint64_t pml4 = (uint64_t)((uint64_t)pages::pml4entries - (uint64_t)WholePhysicalStart);
					amd64::writeTo<volatile uint64_t>(ready + 0, pml4);
					amd64::writeTo<volatile uint64_t>(cpu_id, cpuid);
					amd64::writeTo<volatile uint32_t>(page_table, (uint32_t)((uint64_t)pages::pml4entries - (uint64_t)WholePhysicalStart));
					auto gtdat = (uint32_t)((uint64_t)&globalGdtr - (uint64_t)WholePhysicalStart);
					amd64::writeTo<volatile uint32_t>(page_table + 4, gtdat);
					amd64::writeTo<volatile uint64_t>(stack_start, _stack_start);
					amd64::writeTo<volatile uint64_t>(stack_end, _stack_end);
					amd64::writeTo<volatile uint64_t>(code, (uint64_t)&apStart);

					__sync_synchronize();

					// Send INIT IPI
					{
						uint64_t icr = 0x4500;
						if (x2) {
						} else {
							#define LAPIC_ICRHI           0x0310  // Interrupt Command [63:32]
							#define ICR_DESTINATION_SHIFT 24
							#define LAPIC_ICRLO           0x0300  // Interrupt Command
							#define ICR_INIT              0x00000500
							#define ICR_PHYSICAL          0x00000000
							#define ICR_ASSERT            0x00004000
							#define ICR_EDGE              0x00000000
							#define ICR_NO_SHORTHAND      0x00000000
							#define ICR_SEND_PENDING      0x00001000

							uint32_t apic_id = lapicid;
							localApicOut(LAPIC_ICRHI, apic_id << ICR_DESTINATION_SHIFT);
							localApicOut(LAPIC_ICRLO, ICR_INIT | ICR_PHYSICAL
								| ICR_ASSERT | ICR_EDGE | ICR_NO_SHORTHAND);

								while (localApicIn(LAPIC_ICRLO) & ICR_SEND_PENDING) {};
						}
					}

					volatile auto currentCoresAP = coresAP;

					// Send START IPI
					{
						uint64_t TRAMPOLINE = 0x8000;
						let ap_segment = (TRAMPOLINE >> 12) & (uint64_t)0xFF;
						uint64_t icr = (uint64_t)0x4600 | ap_segment;

						if (x2) {
						} else {
							icr |= ((uint64_t)cpuid) << 56;

							while (localApicIn(0x300) & 1 << 12 == 1 << 12) {}
			   				localApicOut(0x310, (uint32_t)(icr >> 32));
			   				localApicOut(0x300, (uint32_t)icr);
			   				while (localApicIn(0x300) & 1 << 12 == 1 << 12) {}

							uint32_t apic_id = lapicid;
							uint32_t vector = icr;
							#define ICR_STARTUP 0x00000600

							if (0) {
							}
						}
					}

					while (currentCoresAP == coresAP) {
						amd64::pause();
					}

					break;
			}

			data += length;
		}
	}

	static function loadMcfg(const acpi::AcpiMcfg *mcfg) {
		serialPrintf(L"loadMcfg\n");
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
		serialPrintf(L"[PCI] probePci\n");
		for (uint32_t i = 0; i < count; ++i) {
			acpi::PciGroup *pci = &mPci[i];
			for (int32_t bus = pci->busStart; bus <= pci->busEnd; ++bus) {
				for (int32_t dev = 0; dev < 32; ++dev) {
					// TODO
				}
			}
		}
	}
};

volatile uint64_t ACPIParser::coresAP = 1;
Spinlock ACPIParser::apLock;

// Management

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
