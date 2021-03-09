; The Tofita Kernel
; Copyright (C) 2021 Oleg Petrenko
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

global @tofitaFastSystemCallWrapper@8
@tofitaFastSystemCallWrapper@8:
	int 0x80
	ret

extern @greenteaosIsTheBest@8
global __DllMainCRTStartup@12
__DllMainCRTStartup@12:

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
	call @greenteaosIsTheBest@8
