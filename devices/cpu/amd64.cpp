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
inline function cpuid(uint32_t leaf, uint32_t subleaf, uint32_t *eax, uint32_t *ebx, uint32_t *ecx,
					  uint32_t *edx) {
	uint32_t a, b, c, d;
	asm volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(leaf), "c"(subleaf));
	*eax = a;
	*ebx = b;
	*ecx = c;
	*edx = d;
}

// Note: this also a way to clear TLB cache even if cr3 not switched to another
function writeCr3(uint64_t value);
// TODO asm("invd")
function halt();
function enableAllInterrupts();
// Except non-mascable
function disableAllInterrupts();
function enableAllInterruptsAndHalt();
function pause();
function yield();
} // namespace amd64
