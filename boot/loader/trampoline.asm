; The Tofita Kernel
; Copyright (C) 2020 Oleh Petrenko
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

bits 64
default rel
section .head

; UNIX
; rdi - first argument
; rsi - second argument
; rdx - third argument
; rcx - fourth argument
; r8 - fifth argument
; r9 - sixth
; rax - returned value

; Tofita
; rcx - first argument - (uint64_t)params
; rdx - second argument - (uint64_t)paging::pml4entries (CR3)
; r8 - third argument - stack
; r9 - fourth argument - upper (kernel)
; xmm0, xmm1, xmm2, xmm3 for floating point arguments
; rax - returned value

; Tofita 32-bit fastcall
; ecx - first argument
; edx - second argument
; eax - returned value

global trampolineCR3
trampolineCR3:

cli ; Disable interrupts

mov cr3, rdx ; Tofita upper-half paging

mov rsp, r8
push 0 ; Signal end of stack with 0 return address
push 0 ; and a few extra entries in case of stack
push 0 ; problems
push 0
mov rbp, rsp ; Frame

; TODO Reset state
;cld ; Clear direction flag
;; Clear registers
;xor rax, rax
;xor rdx, rdx
;xor rdi, rdi
;xor rsi, rsi
;xor r8, r8
;xor xmm0, xmm0
;xor xmm1, xmm1
;xor xmm2, xmm2
;xor xmm3, xmm3

; Enter main [with rcx]
o64 call r9

; COM write
global portOutb
portOutb:
    mov rax, rdx
    mov rdx, rcx
    out dx, al
    ret

; COM read
global portInb
portInb:
    mov rdx, rcx
    in al, dx
    ret

global binFont
binFont:
incbin "boot/loader/ascii.tofita"

global binFontBitmap
binFontBitmap:
incbin "boot/loader/font.bmp"

global binLeavesBitmap
binLeavesBitmap:
incbin "boot/loader/leaves.bmp"
