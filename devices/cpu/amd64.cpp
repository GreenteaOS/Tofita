// The Tofita Kernel
// Copyright (C) 2020 Oleh Petrenko
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

// AMD64 assembly instructions wrapper

inline function cpuid(uint32_t leaf, uint32_t subleaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx,
					  uint32_t *edx) {
	uint32_t a, b, c, d;
	asm volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(leaf), "c"(subleaf));
	*eax = a;
	*ebx = b;
	*ecx = c;
	*edx = d;
}

// TODO support `enum class` in hexa cpp codegen `enum class MSR : uint32_t {`
enum MSR {
	IA32_APIC_BASE = 27,
};

uint64_t rdmsr(enum MSR addr) {
	uint32_t low, high;
	__asm__ __volatile__ ("rdmsr" : "=a"(low), "=d"(high) : "c"(addr));
	return ((uint64_t)(high) << 32) | low;
	// return (static_cast<uint64_t>(high) << 32) | low;
}

void wrmsr(enum MSR addr, uint64_t value) {
	uint32_t low = value & 0xffffffff;
	uint32_t high = value >> 32;
	__asm__ __volatile__ ("wrmsr" :: "c"(addr), "a"(low), "d"(high));
}

#if 0
extern "C++" template <typename T>
inline T readFrom(uint64_t pointer) {
	return *reinterpret_cast<const T volatile *>(pointer);
}

extern "C++" template <typename T>
inline void writeTo(uint64_t pointer, T value) {
	auto dest = reinterpret_cast<T volatile*>(pointer);
	*dest = value;
}
#endif

// Note: this also a way to clear TLB cache even if cr3 not switched to another
externC function writeCr3(uint64_t value);
// TODO asm("invd")
externC function halt();
externC function enableAllInterrupts();
// Except non-mascable
externC function disableAllInterrupts();
externC function enableAllInterruptsAndHalt();
externC function pause();
externC function yield();
