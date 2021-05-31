; The Tofita Kernel
; Copyright (C) 2021  Oleg Petrenko
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

; Based on https://github.com/redox-os/kernel/blob/47048102efd9961587ca0c6f83c6d93f729e664c/src/asm/x86_64/trampoline.asm

; This code is used to start SMP cores (AP)
; Sets GDT, CPU features, paging, then enters 64-bit mode & jumps to kernel
default abs ; All addresses are relative to 0x00008000
org 0x00008000 ; Known absolute address in NON-virtual memory
section .head ; Required for proper linking, but no sections actually created

%define PADDING 8

%define uint8_t db
%define uint16_t dw
%define uint32_t dd
%define uint64_t dq

; Entry point
; Not exported: entered by absolute address
use16
arguments:
    ; Disable interrupts
    cli
    ; Avoid interpreting arguments by the CPU
    jmp short realModeApStart

    ; Known padding
    times PADDING - ($ - arguments) nop

    ; This memory overwitten directly as a way to pass parameters
    .ready: uint64_t 0 ; TODO Unused
    .cpuIndex: uint64_t 0 ; Current CPU core number
    .pageTable: uint32_t 0 ; PML4
    .padding: uint32_t 0 ; TODO Unused
    .stackStart: uint64_t 0 ; TODO Unused
    .stackEnd: uint64_t 0 ; Stack used by CRT
    .code: uint64_t 0 ; Entry point

; TODO GDT SEGMENTS

; Actual AP entry point body
use16
realModeApStart:
    ; Segment selectors
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax

    ; Stack pointer later set in rsp
    mov sp, 0

    ; Physical pointer to PML4
    ; At this point only 32-bit thus some workarounds required
    mov edi, [arguments.pageTable]
    mov cr3, edi

    ; Enable FPU
    mov eax, cr0
    and al, 11110011b ; Clear task switched (3) and emulation (2)
    or al, 00100010b ; Set numeric error (5) monitor co-processor (1)
    mov cr0, eax

    ; 18: Enable OSXSAVE
    ; 10: Unmasked SSE exceptions
    ; 9: FXSAVE/FXRSTOR
    ; 7: Page Global
    ; 5: Page Address Extension
    ; 4: Page Size Extension
    mov eax, cr4
    or eax, 1 << 18 | 1 << 10 | 1 << 9 | 1 << 7 | 1 << 5 | 1 << 4
    mov cr4, eax

    ; Initialize floating point registers
    fninit

    ; Load protected mode GDT
    lgdt [gdtr]

    ; Read from the EFER MSR
    mov ecx, 0xC0000080
    rdmsr
    ; Set the Long-Mode-Enable and NXE bit
    or eax, 1 << 11 | 1 << 8
    wrmsr

    ; Enable paging and protection simultaneously
    mov ebx, cr0
    ; 31: Paging
    ; 16: Write protect kernel
    ; 0: Protected Mode
    or ebx, 1 << 31 | 1 << 16 | 1
    mov cr0, ebx

    ; Far jump to enable Long Mode and load CS with 64 bit segment
    jmp gdt.kernel_code:longModeApStart

; Now registers and pointers are 64-bit
use64
longModeApStart:
    mov rax, gdt.kernelModeDataSegment
    ; TODO Separate segments for executable memory and data
    mov ds, rax
    mov es, rax
    mov fs, rax
    mov gs, rax
    mov ss, rax

    mov rcx, [arguments.stackEnd]
    lea rsp, [rcx - 256] ; TODO proper stack start

    ; TODO handle ABI properly
    mov rdi, arguments.cpuIndex

    push 0 ; Signal end of stack with 0 return address
    push 0 ; and a few extra entries in case of stack
    push 0 ; problems
    push 0
    mov rbp, rsp ; Frame

    mov rax, [arguments.code]
    o64 call rax
