; The Tofita Kernel
; Copyright (C) 2019  Oleg Petrenko
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

; rdi - first argument
; rsi - second argument
; rdx - third argument
; rcx - fourth argument
; r8 - fifth argument
; r9 - sixth

cli ; Disable interrupts
mov cr3, rsi
o64 jmp rcx
