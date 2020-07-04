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

// SEH exceptions

// Tests in disasm show that this is not optimized out thanks to volatile

volatile bool probeForReadOkay(volatile const uint64_t at, volatile const uint64_t bytes) {
	sehProbe = true;
	// TODO per-page not per-byte
	volatile const uint8_t *ptr = (volatile const uint8_t *)at;
	volatile uint64_t sum = 0;
	for (uint64_t i = 0; i < bytes; ++i) {
		sum += ptr[i];
	}
	sehProbe = false;
	return true;
}

volatile bool probeForWriteOkay(volatile const uint64_t at, volatile const uint64_t bytes) {
	sehProbe = true;
	// TODO per-page not per-byte
	volatile uint8_t *ptr = (volatile uint8_t *)at;
	// TODO i also as volatile?
	for (uint64_t i = 0; i < bytes; ++i) {
		ptr[i] = ptr[i];
	}
	sehProbe = false;
	return true;
}
