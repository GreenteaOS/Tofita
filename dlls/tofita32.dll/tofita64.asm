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

; LLVM generates jmp instead of call
; so we have to use this function in NASM

global tofitaFastSystemCallWrapper
tofitaFastSystemCallWrapper:
	int 0x80
	ret

extern greenteaosIsTheBest
global _DllMainCRTStartup
_DllMainCRTStartup:
; shadow space / red zone
	push 0
	push 0
	push 0
	push 0
	mov rbp, rsp
	call greenteaosIsTheBest
