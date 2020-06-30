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
	process->present = true; // Occupied
	process->pml4 = pages::newCR3(processes[0].pml4);
	process->schedulable = false;						// Not yet ready
	process->scheduleOnNextTick = false;				// Prevent DoS attack
	memset(&process->frame, 0, sizeof(InterruptFrame)); // Zeroing
	memset(&process->stack, 0, sizeof(InterruptStack)); // Zeroing
	process->frame.cs = USER_CODE64_SEL + 3;
	process->frame.ss = USER_DATA32_SEL + 3;
	// process->frame.flags = 0x002; // No interrupts
	process->frame.flags = 0x202;
}

} // namespace process
