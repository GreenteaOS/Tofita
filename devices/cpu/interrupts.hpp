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

struct InterruptFrame {
	// SSE
	volatile uint64_t xmm[8 * 2];

	volatile uint64_t r15;
	volatile uint64_t r14;
	volatile uint64_t r13;
	volatile uint64_t r12;
	volatile uint64_t r11;
	volatile uint64_t r10;
	volatile uint64_t r9;
	volatile uint64_t r8;

	// Note: ECX is RCX, EAX is RAX, etc in 32-bit mode
	volatile uint64_t rdi;
	volatile uint64_t rsi;
	volatile uint64_t rbp;
	volatile uint64_t rbx;
	volatile uint64_t rdx;
	volatile uint64_t rcx;
	volatile uint64_t rax;

	// IRQ index
	volatile uint64_t index;

	// IRQ error code
	volatile uint64_t code;

	volatile uint64_t ip;	 // Instruction pointer
	volatile uint64_t cs;	 // Code segment
	volatile uint64_t flags; // RFLAGS\EFLAGS (same on AMD64 and x86)
	volatile uint64_t sp;	 // Stack pointer
	volatile uint64_t ss;	 // Stack segment
};

_Static_assert(sizeof(InterruptFrame) == 0xb0 + 128, "sizeof is incorrect");
