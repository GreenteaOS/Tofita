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
section .head
%include "devices/cpu/cpu.asm"

; Those fonts used before having access to the disk
global binFont
binFont:
incbin "./boot/loader/ascii.tofita"

global binFontBitmap
binFontBitmap:
incbin "./boot/loader/font.bmp"
