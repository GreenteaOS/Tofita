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

function Process_init(Process *process) {
	process->pml4 = pages::newCR3(processes[0].pml4);
	process->schedulable = false;						// Not yet ready
	process->scheduleOnNextTick = false;				// Prevent DoS attack
	memset(&process->frame, 0, sizeof(InterruptFrame)); // Zeroing
	process->frame.cs = USER_CODE64_SEL + 3;
	process->frame.ss = USER_DATA32_SEL + 3;
	// process->frame.flags = 0x002; // No interrupts
	process->frame.flags = 0x202;
	process->syscallToHandle = TofitaSyscalls::Noop;
	process->frame.rdxArg1 = process->pid; // Second argument
	process->messages = null;
	process->awaitsGetMessage = false;
}

Process *Process_create() {
	uint64_t index = 0;
	while (index < 255) {
		index++;
		if (index == 256)
			return null;
		if (processes[index].present != true)
			break;
	}

	Process *process = &process::processes[index];
	process->pid = index;
	process->present = true; // Occupied

	return process;
}

// Note: 10.000 system-wise on NT
constexpr uint64_t messagesBufferSize = 256;

wapi::Msg *getOrCreateMessagesBuffer(volatile Process *process) {
	if (process->messages == null) {
		process->messages =
			(wapi::Msg *)PhysicalAllocator::allocateBytes(sizeof(wapi::Msg) * messagesBufferSize);
	}

	return process->messages;
}

bool postMessage(volatile Process *process, PostMessagePayload *payload) {
	// TODO do not allocate messages queue if no windows and not awaits for them
	var messages = getOrCreateMessagesBuffer(process);

	if (process->awaitsGetMessage) {
		process->awaitsGetMessage = false;
		process->syscallToHandle = TofitaSyscalls::GetMessage;
	}

	return false;
}

bool getMessage(volatile Process *process, GetMessagePayload *payload) {
	// Do not allocate until needed
	if (process->messages == null)
		return false;

	var messages = getOrCreateMessagesBuffer(process);

	return false;
}

function Process_destroy(Process *process) {
	// TODO deallocate stuff
	process->present = false;
}

} // namespace process
