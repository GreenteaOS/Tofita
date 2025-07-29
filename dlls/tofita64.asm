; The Tofita Engine
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
section .text

; LLVM generates jmp instead of call
; so we have to use this function in NASM

align 16
global tofitaFastSystemCallWrapper
tofitaFastSystemCallWrapper:
	int 0x80
	ret

extern greenteaosIsTheBest
align 16
global _DllMainCRTStartup
_DllMainCRTStartup:
; shadow space / red zone
	mov rbp, 0
	push rbp
	push rbp
	push rbp
	push rbp
	mov rbp, rsp
	call greenteaosIsTheBest

align 16
global binFont
binFont:
incbin "boot/loader/ascii.tofita"

align 16
global binFontBitmap
binFontBitmap:
incbin "boot/loader/font.bmp"

; TODO
align 16
global currentTeb_x64
currentTeb_x64:
    ;mov rax, gs:[0x60]  ; Move the value at GS:[0x60] (TEB address) into RAX
    ret                 ; Return, RAX now contains the TEB address
