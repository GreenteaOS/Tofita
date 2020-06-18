// The Tofita Kernel
// Copyright (C) 2020  Oleg Petrenko
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

namespace efi {
static const efi::EFI_MEMORY_DESCRIPTOR *getNextDescriptor(const efi::EFI_MEMORY_DESCRIPTOR *descriptor, uint64_t descriptorSize) {
	const uint8_t *desc = ((const uint8_t *) descriptor) + descriptorSize;
	return (const efi::EFI_MEMORY_DESCRIPTOR *) desc;
}
}
