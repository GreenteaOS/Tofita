// The Tofita Kernel
// Copyright (C) 2019  Oleg Petrenko
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

// Algorithm from https://fastcpp.blogspot.com/2011/06/bilinear-pixel-interpolation-using-sse.html
PixelRGBAData __attribute__((fastcall)) interpolatePixel(const Bitmap32* bitmap, float x, float y) {
	uint16_t px = (uint16_t)x; // Same as floor(x)
	uint16_t py = (uint16_t)y; // Same as floor(y)
	const uint16_t stride = bitmap->width;
	const PixelRGBAData* p0 = bitmap->pixels + px + py * stride; // Pointer to first pixel

	// Calculate the weights
	float fx = x - px;
	float fy = y - py;
	float fx1 = 1.0f - fx;
	float fy1 = 1.0f - fy;

	uint32_t w1 = fx1 * fy1 * 256.0f;
	uint32_t w2 = fx  * fy1 * 256.0f;
	uint32_t w3 = fx1 * fy  * 256.0f;
	uint32_t w4 = fx  * fy  * 256.0f;

	PixelRGBAData result;

	// Neighboring pixels
	const PixelRGBAData p1 = p0[0         ];
	const PixelRGBAData p2 = p0[1         ];
	const PixelRGBAData p3 = p0[    stride];
	const PixelRGBAData p4 = p0[1 + stride];

	// Weighted sum of pixels
	result.r = (p1.r * w1 + p2.r * w2 + p3.r * w3 + p4.r * w4) >> 8;
	result.g = (p1.g * w1 + p2.g * w2 + p3.g * w3 + p4.g * w4) >> 8;
	result.b = (p1.b * w1 + p2.b * w2 + p3.b * w3 + p4.b * w4) >> 8;
	result.a = (p1.a * w1 + p2.a * w2 + p3.a * w3 + p4.a * w4) >> 8;

	return result;
}
