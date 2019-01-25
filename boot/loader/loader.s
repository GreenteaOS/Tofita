# The Tofita Kernel
# Copyright (C) 2019  Oleg Petrenko
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as published by
# the Free Software Foundation, version 3 of the License.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

.section .head
mov $stack_top, %rsp
call startTofitaKernelLoader
ret

.section .stack
.align 16
stack_bottom:
.skip 0x4000 # 16 KiB
stack_top:
