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

// AMD64 assembly instructions wrapper

namespace amd64 {
inline function cpuid(uint32_t leaf, uint32_t subleaf, uint32_t *eax,
					  uint32_t *ebx = null, uint32_t *ecx = null,
					  uint32_t *edx = null) {
	uint32_t a, b, c, d;
	asm volatile("cpuid"
				 : "=a"(a), "=b"(b), "=c"(c), "=d"(d)
				 : "a"(leaf), "c"(subleaf));
	if (eax)
		*eax = a;
	if (ebx)
		*ebx = b;
	if (ecx)
		*ecx = c;
	if (edx)
		*edx = d;
}

inline function writeCr3(uint64_t value) {
	asm volatile("movq %0, %%cr3" :: "r"(value));
}
} // namespace amd64
