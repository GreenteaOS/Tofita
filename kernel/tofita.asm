; The Tofita Kernel
; Copyright (C) 2019  Oleg Petrenko
;
; This program is free software: you can redistribute it and/or modify
; it under the terms of the GNU Lesser General Public License as published by
; the Free Software Foundation, version 3 of the License.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
; GNU Lesser General Public License for more details.
;
; You should have received a copy of the GNU Lesser General Public License
; along with this program.  If not, see <https://www.gnu.org/licenses/>.

section .head1
_start:
	cli ; Disable interrupts

	mov rsp, stack_end
	push 0 ; Signal end of stack with 0 return address
	push 0 ; and a few extra entries in case of stack
	push 0 ; problems
	push 0

	mov rbp, rsp

	extern kernelMain
	call kernelMain
.halt: ; Don't waste CPU
	hlt
	jmp .halt

; This section is probably useless
section .stack1
align 16
stack_bottom:
	resb 0x10000 ; 64 KiB stack space
stack_top:

section .bss
align 0x100
stack_begin:
	resb 0x10000 ; 64 KiB stack space
stack_end:
