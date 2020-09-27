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

uint64_t String::length(const wchar_t *source) {
	let string = (const uint16_t *)source;
	uint32_t i = 0;
	while (string[i] != 0) {
		i++;
	}
	return i * 2 + 2;
}

const wchar_t *String::duplicate(const wchar_t *source) {
	let bytes = length(source);
	var result = (uint16_t *)allocateBytes(bytes);
	let string = (const uint16_t *)source;

	uint32_t i = 0;
	while (string[i] != 0) {
		result[i] = string[i];
		i++;
	}

	return (const wchar_t *)result;
}
