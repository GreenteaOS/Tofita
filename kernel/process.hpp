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

struct Process {
	// Is process exists at all and fully initialized
	bool present;

	// Is it allowed to make this process current
	bool schedulable;

	// Should be false after creation
	bool scheduleOnNextTick;

	// CR3, should be page aligned
	pages::PageEntry *pml4;

	// TODO 32-bit

	// State
	InterruptFrame frame;
	InterruptStack stack;
};

// TODO dynamic allocation of this list
Process processes[256] = {0};

uint64_t currentProcess = 0;

} // namespace process
