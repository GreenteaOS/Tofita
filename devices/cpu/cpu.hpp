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

// Bindings to the .asm

extern "C" function enterKernelMode();
function selectSegment();
function enterUserMode();
extern "C" function timerInterrupt();
extern "C" function syscallInterrupt();
extern "C" function spuriousInterrupt();
extern "C" function yieldInterrupt();
extern "C" volatile uint64_t getCr2();
extern "C" volatile bool sehReturnFalse();

extern "C" function cpu0x00();
extern "C" function cpu0x01();
extern "C" function cpu0x02();
extern "C" function cpu0x03();
extern "C" function cpu0x04();
extern "C" function cpu0x05();
extern "C" function cpu0x06();
extern "C" function cpu0x07();
extern "C" function cpu0x08();
extern "C" function cpu0x09();
extern "C" function cpu0x0A();
extern "C" function cpu0x0B();
extern "C" function cpu0x0C();
extern "C" function cpu0x0D();
extern "C" function cpu0x0E();
extern "C" function cpu0x0F();
extern "C" function cpu0x10();
extern "C" function cpu0x11();
extern "C" function cpu0x12();
extern "C" function cpu0x13();
extern "C" function cpu0x14();
extern "C" function cpu0x15();
