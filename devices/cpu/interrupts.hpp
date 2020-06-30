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
	uint64_t ip; // Instruction pointer
	uint64_t cs; // Code segment
	uint64_t flags;
	uint64_t sp; // Stack pointer
	uint64_t ss; // Stack segment
} __attribute__((packed));

struct InterruptStack {
	uint64_t extra[6]; // TODO investigate (probably InterruptFrame itself + 8 byte offset)

	uint64_t xmm[6 * 2]; // 16-byte spill

	uint64_t rcx;
	uint64_t rdx;
	uint64_t r8;
	uint64_t r9;
	uint64_t r10;
	uint64_t r11;
	uint64_t rax;
} __attribute__((packed));

_Static_assert(sizeof(InterruptStack) == 200, "sizeof is incorrect");
