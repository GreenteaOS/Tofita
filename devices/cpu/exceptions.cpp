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

// CPU exceptions

void exceptionHandler(InterruptFrame *const frame) {
	// Page fault
	volatile uint64_t cr2 = 0;
	if (frame->index == 0x0E)
		cr2 = getCr2();

	if (currentThread == THREAD_USER) {
		volatile process::Process *process = &process::processes[process::currentProcess];
		process->schedulable = false;
		process->syscallToHandle = TofitaSyscalls::Cpu;
		process->cr2PageFaultAddress = cr2;
		switchToKernelThread(frame);
	} else if (currentThread == THREAD_GUI) {
		frame->ip = (uint64_t)&kernelThreadLoop;
		volatile process::Process *process = &process::processes[0];
		process->cr2PageFaultAddress = cr2;
		qsod(L"THREAD_GUI unhandled CPU exception: index = %u, code = %8\n", frame->index, frame->code);
	} else if (currentThread != THREAD_USER) {
		// SEH
		if (sehProbe) {
			frame->ip = (uint64_t)&sehReturnFalse;
			sehProbe = false;
			return;
		}

		frame->ip = (uint64_t)&kernelThreadLoop;
		volatile process::Process *process = &process::processes[0];
		process->cr2PageFaultAddress = cr2;
		qsod(L"NON-THREAD_USER unhandled CPU exception: index = %u, code = %8\n", frame->index, frame->code);
	}
}
