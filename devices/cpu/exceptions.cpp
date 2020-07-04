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
	if (currentThread == THREAD_USER) {
		volatile process::Process *process = &process::processes[process::currentProcess];
		process->schedulable = false;
		process->syscallToHandle = TofitaSyscalls::Cpu;
		switchToKernelThread(frame);
	} else if (currentThread == THREAD_GUI) {
		frame->ip = (uint64_t)&kernelThreadLoop;
		qsod(u8"THREAD_GUI unhandled CPU exception: index = %u, code = %8\n", frame->index, frame->code);
	} else if (currentThread == THREAD_KERNEL) {
		frame->ip = (uint64_t)&kernelThreadLoop;
		qsod(u8"THREAD_KERNEL unhandled CPU exception: index = %u, code = %8\n", frame->index, frame->code);
	}
}
