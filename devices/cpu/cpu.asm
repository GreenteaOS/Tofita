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
%define USER_CODE32_SEL 0x20
%define USER_DATA32_SEL 0x28
%define USER_CODE64_SEL 0x30
%define SYS_CODE32_SEL 0x60

; Note in this case | 3 same as + 3
%define DATA (USER_DATA32_SEL + 3)
%define CODE (USER_CODE64_SEL + 3)
%define CODEWoW (USER_CODE32_SEL + 3)

global enterUserModeWoW; 32-bit mode TODO CODEWoW
global enterUserMode; 64-bit mode
enterUserMode:
	cli

	mov ax, DATA
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov rax, rsp
	push DATA; ss
	push rax; rsp
	pushfq; rflags
	or dword [rsp], 0b1000000000; IF - interrupt enable flag
	push CODE; cs
	mov rax, qword .ret; rip
	push rax
	iretq
.ret:
	mov rax, 101
	mov r11, 102
	mov r10, 103
	mov r9, 104
	mov r8, 105
	mov rdx, 106
	mov rcx, 107
	int3
	jmp $
	ret

extern guiThread
global guiThreadStart_
guiThreadStart_:
	push 0 ; Signal end of stack with 0 return address
	push 0 ; and a few extra entries in case of stack
	push 0 ; problems
	push 0
	mov rbp, rsp ; Frame
	o64 call guiThread

extern kernelThread
global kernelThreadStart_
kernelThreadStart_:
	push 0 ; Signal end of stack with 0 return address
	push 0 ; and a few extra entries in case of stack
	push 0 ; problems
	push 0
	mov rbp, rsp ; Frame
	o64 call kernelThread

global enterKernelMode
enterKernelMode:
	cli

	mov ax, SYS_DATA32_SEL
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	mov rcx, rsp
	push SYS_DATA32_SEL; ss
	push rcx; rsp
	pushfq; rflags
	;or dword [rsp], 0b1000000000; IF - interrupt enable flag
	push SYS_CODE64_SEL; cs
	mov     rcx, qword .ret; rip
	push    rcx
	iretq
.ret:
	ret

global selectSegment
; TODO set COMPAT_SEL as func agrument
; TODO set also SS
; TODO move this into C code inline asm
%define retfq o64 retf
selectSegment:
	push SYS_CODE64_SEL ; push GDT selector
	mov rcx, qword .ret; rip
	push rcx
	retfq
.ret:
	ret

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

; This is just a type declaration
struc InterruptFrame
.xmm0     resq 2
.xmm1     resq 2
.xmm2     resq 2
.xmm3     resq 2
.xmm4     resq 2
.xmm5     resq 2
.xmm6     resq 2
.xmm7     resq 2

.r15     resq 1
.r14     resq 1
.r13     resq 1
.r12     resq 1
.r11     resq 1
.r10     resq 1
.r9      resq 1
.r8      resq 1

.rdi     resq 1
.rsi     resq 1
.rbp     resq 1
.rbx     resq 1
.rdx     resq 1
.rcx     resq 1
.rax     resq 1

.int     resq 1
.err     resq 1
.rip     resq 1
.cs3     resq 1
.rflags  resq 1
.rsp3    resq 1
.ss3     resq 1
endstruc

registerStorageSize equ (0x78 + 128)

%macro pushAllRegisters 0
	sub rsp, registerStorageSize

	mov [rsp + InterruptFrame.rax], rax
	mov [rsp + InterruptFrame.rcx], rcx
	mov [rsp + InterruptFrame.rdx], rdx
	mov [rsp + InterruptFrame.rbx], rbx
	mov [rsp + InterruptFrame.rbp], rbp
	mov [rsp + InterruptFrame.rsi], rsi
	mov [rsp + InterruptFrame.rdi], rdi

	mov [rsp + InterruptFrame.r8 ], r8
	mov [rsp + InterruptFrame.r9 ], r9
	mov [rsp + InterruptFrame.r10], r10
	mov [rsp + InterruptFrame.r11], r11
	mov [rsp + InterruptFrame.r12], r12
	mov [rsp + InterruptFrame.r13], r13
	mov [rsp + InterruptFrame.r14], r14
	mov [rsp + InterruptFrame.r15], r15

	; SSE registers
	movaps [rsp + InterruptFrame.xmm0], xmm0
	movaps [rsp + InterruptFrame.xmm1], xmm1
	movaps [rsp + InterruptFrame.xmm2], xmm2
	movaps [rsp + InterruptFrame.xmm3], xmm3
	movaps [rsp + InterruptFrame.xmm4], xmm4
	movaps [rsp + InterruptFrame.xmm5], xmm5
	movaps [rsp + InterruptFrame.xmm6], xmm6
	movaps [rsp + InterruptFrame.xmm7], xmm7
%endmacro

%macro popAllRegisters 0
	mov rax, [rsp + InterruptFrame.rax]
	mov rcx, [rsp + InterruptFrame.rcx]
	mov rdx, [rsp + InterruptFrame.rdx]
	mov rbx, [rsp + InterruptFrame.rbx]
	mov rbp, [rsp + InterruptFrame.rbp]
	mov rsi, [rsp + InterruptFrame.rsi]
	mov rdi, [rsp + InterruptFrame.rdi]

	mov r8,  [rsp + InterruptFrame.r8 ]
	mov r9,  [rsp + InterruptFrame.r9 ]
	mov r10, [rsp + InterruptFrame.r10]
	mov r11, [rsp + InterruptFrame.r11]
	mov r12, [rsp + InterruptFrame.r12]
	mov r13, [rsp + InterruptFrame.r13]
	mov r14, [rsp + InterruptFrame.r14]
	mov r15, [rsp + InterruptFrame.r15]

	; SSE registers
	movaps xmm0, [rsp + InterruptFrame.xmm0]
	movaps xmm1, [rsp + InterruptFrame.xmm1]
	movaps xmm2, [rsp + InterruptFrame.xmm2]
	movaps xmm3, [rsp + InterruptFrame.xmm3]
	movaps xmm4, [rsp + InterruptFrame.xmm4]
	movaps xmm5, [rsp + InterruptFrame.xmm5]
	movaps xmm6, [rsp + InterruptFrame.xmm6]
	movaps xmm7, [rsp + InterruptFrame.xmm7]

	add rsp, registerStorageSize
%endmacro

%macro swapGsIfRequired 0
	mov rax, [rsp+0x90]
	and rax, 0x03
	cmp rax, 0x03
	jne %%noSwapGs
	swapgs
%%noSwapGs:
%endmacro

%macro defineIRQHandler 2
extern %1
global %2
%2:
	pushAllRegisters
	swapGsIfRequired

	mov rcx, rsp
	call %1
	jmp irqHandlerReturn
%endmacro

global irqHandlerReturn
irqHandlerReturn:
	swapGsIfRequired
	popAllRegisters

	add rsp, 16
	iretq

%macro defineIRQ 3
	global %2
	%2:
		push 0
		push %1
		jmp %3
%endmacro

defineIRQHandler timerInterruptHandler, timerInterruptPrelude
defineIRQ 0x82, timerInterrupt, timerInterruptPrelude

defineIRQHandler syscallInterruptHandler, syscallInterruptPrelude
defineIRQ 0x82, syscallInterrupt, syscallInterruptPrelude

defineIRQHandler yieldInterruptHandler, yieldInterruptPrelude
defineIRQ 0x82, yieldInterrupt, yieldInterruptPrelude

%macro fallback 1
	fallback_handler%1:
		iretq
%endmacro

keyboardHandler:
	iretq

mouseHandler:
	iretq

