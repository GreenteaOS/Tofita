; The Tofita Kernel
; Copyright (C) 2021-2023 Oleh Petrenko
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

bits 32
default rel
section .head
; TODO use .text section? This thing creates unnecessary .head section in .dll

global @tofitaFastSystemCallWrapper@8
@tofitaFastSystemCallWrapper@8:
	; TODO set stack frame for debuggers?
	; TODO inline this call into calling functions themselves?
	; TODO is it called with fastcall for sure? @8?
	int 0x80
	ret

extern greenteaosIsTheBest
global __DllMainCRTStartup@12
__DllMainCRTStartup@12:
; shadow space / red zone
; TODO does 32-bit need any ot these?
; TODO set offset on PE loader size probably,
; also compiler should allow calling this without wrappers
	push 0
	push 0
	push 0
	push 0

	push 0
	push 0
	push 0
	push 0
	push ebp
	mov ebp, esp
	call greenteaosIsTheBest
