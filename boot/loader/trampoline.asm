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

; NT
; rcx - first argument - (uint64_t)params
; rdx - second argument - (uint64_t)paging::pml4entries (CR3)
; r8 - third argument - stack
; r9 - fourth argument - upper (kernel)
; xmm0, xmm1, xmm2, xmm3 for floating point arguments
; rax - returned value

global trampolineCR3
trampolineCR3:
cli ; Disable interrupts
;mov cr3, rsi ; UNIX
;o64 jmp rcx ; UNIX

mov cr3, rdx ; NT

mov rsp, r8
push 0 ; Signal end of stack with 0 return address
push 0 ; and a few extra entries in case of stack
push 0 ; problems
push 0
mov rbp, rsp ; Frame

o64 call r9
