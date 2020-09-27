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

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
	uint8_t a;
} __attribute__((packed)) PixelRGBAData;

struct Pixel32 {
	union {
		PixelRGBAData rgba;
		uint32_t color;
	};
};

_Static_assert(sizeof(Pixel32) == 4, "bad sizeof");

struct Bitmap32Interim {
	uint16_t width;
	uint16_t height;
	Pixel32 pixels[];
};

struct Bitmap32 {
	const uint16_t width;
	const uint16_t height;
	Pixel32 pixels[];
};
