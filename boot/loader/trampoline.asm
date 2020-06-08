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

cli ; Disable interrupts
;mov cr3, rsi ; UNIX
;o64 jmp rcx ; UNIX

mov cr3, rdx ; NT
mov rdi, rcx ; NT -> UNIX
mov rdx, r8 ; NT -> UNIX
o64 jmp r9 ; NT
