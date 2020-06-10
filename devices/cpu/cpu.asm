; The Tofita Kernel
; Copyright (C) 2020  Oleg Petrenko
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

%define SYS_CODE64_SEL 0x10
%define SYS_DATA32_SEL 0x18

global enterUserMode
enterUserMode:
	cli
	mov rax, rsp
	push SYS_DATA32_SEL; ss
    push rax; rsp
    pushfq; rflags
    push SYS_CODE64_SEL; cs
    mov     rax, qword .ret; rip
    push    rax
    iretq
.ret:
	sti; TODO AFAIK should be set in rflags
	ret

global selectSegment
; TODO set COMPAT_SEL as func agrument
; TODO set also SS
; TODO move this into C code inline asm
%define retfq o64 retf
selectSegment:
	push SYS_CODE64_SEL ; push GDT selector
	push rdi
	retfq

; exports
global keyboardHandler
global mouseHandler

global fallback_handler0
global fallback_handler1
global fallback_handler2
global fallback_handler3
global fallback_handler4
global fallback_handler5
global fallback_handler6
global fallback_handler7
global fallback_handler8
global fallback_handler9
global fallback_handler10
global fallback_handler11
global fallback_handler12
global fallback_handler13
global fallback_handler14
global fallback_handler15

;%macro write_string 2
;	mov eax, 4
;	mov ebx, 1
;	mov ecx, %1
;	mov edx, %2
;	int 80h
;%endmacro

%macro save_regs 0
	push gs
	push fs
	push rbp
	push r15
	push r14
	push r13
	push r12
	push r11
	push r10
	push r9
	push r8
	push rdi
	push rsi
	push rdx
	push rcx
	push rbx
	push rax
	xor  rax, rax
	mov  ds, ax
	push rax
	mov  es, ax
	push rax
%endmacro

%macro restore_regs 0
	pop rax
	mov ax, es
	pop rax
	mov ax, ds
	pop rax
	pop rbx
	pop rcx
	pop rdx
	pop rsi
	pop rdi
	pop r8
	pop r9
	pop r10
	pop r11
	pop r12
	pop r13
	pop r14
	pop r15
	pop rbp
	pop fs
	pop gs
%endmacro

%macro fallback 1
	fallback_handler%1:
		iretq
%endmacro

keyboardHandler:
	iretq

mouseHandler:
	iretq

