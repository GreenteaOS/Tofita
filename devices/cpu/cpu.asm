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

.index     resq 1
.code     resq 1
.ip     resq 1
.cs     resq 1
.flags  resq 1
.sp    resq 1
.ss     resq 1
.fs     resq 1
.gs     resq 1
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
	mov rax, [rsp + InterruptFrame.cs]
	and rax, 0x03
	cmp rax, 0x03
	jne .noSwapGs
	swapgs

	; TODO save/restore DS and ES, etc?
	mov ds, [rsp + InterruptFrame.ss]
	mov fs, [rsp + InterruptFrame.fs]
	mov gs, [rsp + InterruptFrame.gs]
.noSwapGs:
%endmacro

%macro defineIRQHandler 2
extern %1
global %2
%2:
	pushAllRegisters
	swapGsIfRequired

	mov rcx, rsp
	push 0
	push 0
	push 0
	push 0
	mov	rbp, rsp
	call %1
	pop rcx
	pop rcx
	pop rcx
	pop rcx
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
		push 0; Fill error field
		push %1; TODO single global handler?
		jmp %3
%endmacro

%macro defineIRQwithErrCode 3
	global %2
	%2:
		; Error is pushed onto the stack
		push %1; TODO single global handler?
		jmp %3
%endmacro

%macro defineIRQnoCode 4
	defineIRQHandler %4, %2Prelude
	defineIRQ %1, %2, %2Prelude
%endmacro

%macro defineIRQwithCode 4
	defineIRQHandler %4, %2Prelude
	defineIRQwithErrCode %1, %2, %2Prelude
%endmacro

defineIRQnoCode 0x00, timerInterrupt, timerInterruptPrelude, timerInterruptHandler_
defineIRQnoCode 0x80, syscallInterrupt, syscallInterruptPrelude, syscallInterruptHandler_
defineIRQnoCode 0x00, yieldInterrupt, yieldInterruptPrelude, yieldInterruptHandler_
defineIRQnoCode 0x00, keyboardHandler, keyboardInterruptPrelude, handleKeyboard_
defineIRQnoCode 0x00, mouseHandler, mouseInterruptPrelude, handleMouse_

global spuriousInterrupt
spuriousInterrupt:
	iretq
	nop

; CPU exceptions
defineIRQHandler exceptionHandler_, exceptionPrelude

defineIRQ 0x00, cpu0x00, exceptionPrelude ; Division by zero
defineIRQ 0x01, cpu0x01, exceptionPrelude ; Single-step interrupt (see trap flag)
defineIRQ 0x02, cpu0x02, exceptionPrelude ; NMI Interrupt
defineIRQ 0x03, cpu0x03, exceptionPrelude ; Breakpoint (callable by the special 1-byte instruction 0xCC (INT3), used by debuggers)
defineIRQ 0x04, cpu0x04, exceptionPrelude ; Overflow (INTO)
defineIRQ 0x05, cpu0x05, exceptionPrelude ; Bounds range exceeded (BOUND)
defineIRQ 0x06, cpu0x06, exceptionPrelude ; Invalid Opcode (UD2)
defineIRQ 0x07, cpu0x07, exceptionPrelude ; Device/Coprocessor not available (WAIT/FWAIT)
defineIRQwithErrCode 0x08, cpu0x08, exceptionPrelude ; Double fault
defineIRQ 0x09, cpu0x09, exceptionPrelude ; Coprocessor Segment Overrun (386 or earlier only)
defineIRQwithErrCode 0x0A, cpu0x0A, exceptionPrelude ; Invalid Task State Segment
defineIRQwithErrCode 0x0B, cpu0x0B, exceptionPrelude ; Segment not present
defineIRQwithErrCode 0x0C, cpu0x0C, exceptionPrelude ; Stack-segment Fault
defineIRQwithErrCode 0x0D, cpu0x0D, exceptionPrelude ; General protection fault #GP
defineIRQwithErrCode 0x0E, cpu0x0E, exceptionPrelude ; Page fault #PF
defineIRQ 0x0F, cpu0x0F, exceptionPrelude ; reserved
defineIRQ 0x10, cpu0x10, exceptionPrelude ; Math Fault (x87 FPU error)
defineIRQwithErrCode 0x11, cpu0x11, exceptionPrelude ; Alignment Check
defineIRQ 0x12, cpu0x12, exceptionPrelude ; Machine Check
defineIRQ 0x13, cpu0x13, exceptionPrelude ; SIMD Floating-Point Exception
defineIRQ 0x14, cpu0x14, exceptionPrelude ; Virtualization Exception
defineIRQ 0x15, cpu0x15, exceptionPrelude ; Control Protection Exception

global getCr2
getCr2:
	mov rax, cr2
	ret

global sehReturnFalse
sehReturnFalse:
	mov al, 0; bool
	add	rsp, 16; Check disasm of probeFor*
	ret

global halt
halt:
	hlt
	ret

global enableAllInterrupts
enableAllInterrupts:
	sti
	ret

global disableAllInterrupts
disableAllInterrupts:
	cli
	ret

global enableAllInterruptsAndHalt
enableAllInterruptsAndHalt:
	sti
	hlt
	ret

global writeCr3
writeCr3:
	mfence
	mov	cr3, rcx
	ret

global pause
pause:
	pause
	ret

global yield
yield:
	int 0x81
	ret

extern guiThread_
global guiThreadStart
guiThreadStart:
	push 0
	push 0
	push 0
	push 0
	mov rbp, rsp
	call guiThread_
	; o64 call guiThread_ ; Alternatively

extern kernelThread_
global kernelThreadStart
kernelThreadStart:
	push 0
	push 0
	push 0
	push 0
	mov rbp, rsp
	call kernelThread_
	; o64 call kernelThread_ ; Alternatively

global portOutb
portOutb:
    mov rax, rdx
    mov rdx, rcx
    out dx, al
    ret

global portInb
portInb:
    mov rdx, rcx
    in al, dx
    ret

global setTsr
setTsr:
    mov rax, rcx
    ltr ax
    ret

global lgdt
lgdt:
    lgdt [rcx]
    ret

global loadIdt
loadIdt:
    lidt [rcx]
    ret

global rdtscTicks
rdtscTicks:
    rdtsc
    shl rdx, 32
    or rax, rdx
    mov qword [rcx], rax
    ret
