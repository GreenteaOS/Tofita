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

namespace process {

// TODO probably use Thread instead of Process for easier scheduling & syscalls
// and threads will have parentPID and that's
// so we don't have linked list of threads per process
struct Process {
	// Is process exists at all and fully initialized
	volatile bool present;
	// TODO SoA

	// Index
	volatile uint64_t pid;

	// Is it allowed to make this process current
	volatile bool schedulable;
	// TODO combine booleans into bit/flag mask?

	// Should be false after creation
	volatile bool scheduleOnNextTick;

	// CR3, should be page aligned
	pages::PageEntry *volatile pml4;

	// TODO 32-bit

	// State
	InterruptFrame frame;

	// Filled on #PF
	volatile uint64_t cr2PageFaultAddress;

	// When syscall happens, process halted and handled retroactively
	// in the kernel's event loop
	volatile TofitaSyscalls volatile syscallToHandle; // = Noop if no actions required
													  // TODO do this on per-thread basis

	// TODO separate syscall arguments, cause they differ in 32-bit
	// TODO syscall *optional* return value? not every syscall does
	// oh probably

	// Ring buffer
	wapi::Msg *messages;
	uint64_t messagesUsed;
	bool awaitsGetMessage;
	// TODO bool wmQuit/wmPaint for lowest-priority must-never-lost messages
};

constexpr uint64_t ProcessLimit = 256;

// TODO dynamic allocation of this list
Process processes[ProcessLimit] = {0};

// Working process pid, with restored state and active CR3
volatile uint64_t currentProcess = 0;

bool postMessage(volatile Process *process, PostMessagePayload *payload);
bool getMessage(volatile Process *process, GetMessagePayload *payload);
} // namespace process
