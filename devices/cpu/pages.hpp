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

#define PACKED __attribute__((gcc_struct, packed))

namespace pages {

enum class AddressAwareness : uint8_t {
	Bit32limit2GB = 0x0000, // Default
	Bit32limit3GB = 0x0001, // Large address aware (compatibility mode)
	Bit32limit4GB = 0x0002, // Large address aware on 64-bit kernels
	Bit64limit8TB = 0x0003, // Default
	Bit64limit128TB = 0x0004, // Default on newer kernels (compatibility mode)
};

typedef struct {
	// Is the page present in physical memory?
	uint8_t present : 1;

	// Pages are read-only by default
	uint8_t writeAllowed : 1;

	// Pages are only accessible by supervisor by default
	uint8_t accessibleByAll : 1;

	// Write through abilities of the page
	uint8_t writeThrough : 1;

	// If set, the page will not be cached
	uint8_t cacheDisabled : 1;

	// Was the page accessed?
	uint8_t accessed : 1;

	// Has the page been written to? Only applicable for PTE.
	uint8_t dirty : 1;

	// Page size by default is small, enabling this bit makes it bigger. Only applicable for PTE
	uint8_t largePage : 1;

	// Prevent the translations cache from updating
	// the address in cache if CR3 is reset.
	//
	// Page global enable bit in CR4 has to be set to
	// enable this feature.
	//
	// Only applicable for PTE
	uint8_t global : 1;

	// Not used by the processor
	uint8_t metadata : 3;

	// Physical address of the child table/page
	uint64_t address : 40;

	// Not used by the processor
	uint8_t metadata2 : 7;

	// Only applicable for PTE
	uint8_t protectionKey : 4;

	// Disable execution of code from this page
	uint8_t disableExecution : 1;
} PACKED PageEntry;

_Static_assert(sizeof(PageEntry) == sizeof(uint64_t), "page entry has to be 64 bits");
} // namespace pages
