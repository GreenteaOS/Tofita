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

// Based on the http://blog.ivank.net/fastest-gaussian-blur.html

int __attribute__((fastcall)) floor(float x) {
	if (x >= 0) {
		return (int)x;
	} else {
		int y = (int)x;
		return ((float)y == x) ? y : y - 1;
	}
}

float __attribute__((fastcall)) sqrt(float number) {
	long i;
	float x2, y;
	const float threehalfs = 1.5F;

	x2 = number * 0.5F;
	y  = number;
	i  = * ( long * ) &y;                     // floating point bit level hacking [sic]
	i  = 0x5f3759df - ( i >> 1 );             // Newton's approximation
	y  = * ( float * ) &i;
	y  = y * ( threehalfs - ( x2 * y * y ) ); // 1st iteration
	y  = y * ( threehalfs - ( x2 * y * y ) ); // 2nd iteration
	y  = y * ( threehalfs - ( x2 * y * y ) ); // 3rd iteration

	return 1.0f / y;
}

void __attribute__((fastcall)) boxBlurT(Bitmap32 *source, Bitmap32 *target, uint16_t width, uint16_t height, uint8_t radius) {
	double iarr = (double)1.0 / (radius + radius + 1);
	for (uint16_t i = 0; i < width; i++) {
		uint32_t ti = i;
		uint32_t li = ti;
		uint32_t ri = ti + radius * width;

		PixelRGBAData fv = source->pixels[ti].rgba;
		PixelRGBAData lv = source->pixels[ti + width * (height - 1)].rgba;

		uint16_t val_r = fv.r * (radius + 1);
		uint16_t val_g = fv.g * (radius + 1);
		uint16_t val_b = fv.b * (radius + 1);

		for (uint8_t j = 0; j < radius; j++) {
			PixelRGBAData pixel = source->pixels[ti + j * width].rgba;
			val_r += pixel.r;
			val_g += pixel.g;
			val_b += pixel.b;
		}

		for (uint8_t j = 0; j <= radius; j++) {
			PixelRGBAData pixel = source->pixels[ri].rgba;
			val_r += (pixel.r - fv.r);
			val_g += (pixel.g - fv.g);
			val_b += (pixel.b - fv.b);

			target->pixels[ti].rgba.r = val_r * iarr;
			target->pixels[ti].rgba.g = val_g * iarr;
			target->pixels[ti].rgba.b = val_b * iarr;

			ri += width;
			ti += width;
		}

		for (uint8_t j = radius + 1; j < height - radius; j++) {
			PixelRGBAData pixel_ri = source->pixels[ri].rgba;
			PixelRGBAData pixel_li = source->pixels[li].rgba;

			val_r += (pixel_ri.r - pixel_li.r);
			val_g += (pixel_ri.g - pixel_li.g);
			val_b += (pixel_ri.b - pixel_li.b);

			target->pixels[ti].rgba.r = val_r * iarr;
			target->pixels[ti].rgba.g = val_g * iarr;
			target->pixels[ti].rgba.b = val_b * iarr;

			li += width;
			ri += width;
			ti += width;
		}

		for (uint16_t j = height - radius; j < height; j++) {
			PixelRGBAData pixel = source->pixels[li].rgba;

			val_r += (lv.r + pixel.r);
			val_g += (lv.g + pixel.g);
			val_b += (lv.b + pixel.b);

			target->pixels[ti].rgba.r = val_r * iarr;
			target->pixels[ti].rgba.g = val_g * iarr;
			target->pixels[ti].rgba.b = val_b * iarr;

			li += width;
			ti += width;
		}
	}
}

void __attribute__((fastcall)) boxBlurH(Bitmap32 *source, Bitmap32 *target, uint16_t width, uint16_t height, uint8_t radius) {
	double iarr = (double)1.0 / (radius + radius + 1);
	for (uint16_t i = 0; i < height; i++) {
		uint32_t ti = i * width;
		uint32_t li = ti;
		uint32_t ri = ti + radius;
		PixelRGBAData fv = source->pixels[ti].rgba;
		PixelRGBAData lv = source->pixels[ti + width - 1].rgba;

		uint16_t val_r = fv.r * (radius + 1);
		uint16_t val_g = fv.g * (radius + 1);
		uint16_t val_b = fv.b * (radius + 1);

		for (uint8_t j = 0; j < radius; j++) {
			PixelRGBAData pixel = source->pixels[ti + j].rgba;
			val_r += pixel.r;
			val_g += pixel.g;
			val_b += pixel.b;
		}

		for (uint8_t j = 0; j <= radius; j++) {
			PixelRGBAData pixel = source->pixels[ri++].rgba;
			val_r += (pixel.r - fv.r);
			val_g += (pixel.g - fv.g);
			val_b += (pixel.b - fv.b);

			target->pixels[ti].rgba.r = val_r * iarr;
			target->pixels[ti].rgba.g = val_g * iarr;
			target->pixels[ti].rgba.b = val_b * iarr;
			ti++;
		}

		for (uint8_t j = radius + 1; j < width - radius; j++) {
			PixelRGBAData pixel_ri = source->pixels[ri++].rgba;
			PixelRGBAData pixel_li = source->pixels[li++].rgba;

			val_r += (pixel_ri.r - pixel_li.r);
			val_g += (pixel_ri.g - pixel_li.g);
			val_b += (pixel_ri.b - pixel_li.b);

			target->pixels[ti].rgba.r = val_r * iarr;
			target->pixels[ti].rgba.g = val_g * iarr;
			target->pixels[ti].rgba.b = val_b * iarr;
			ti++;
		}

		for (uint16_t j = width - radius; j < width; j++) {
			PixelRGBAData pixel = source->pixels[li++].rgba;

			val_r += (lv.r - pixel.r);
			val_g += (lv.g - pixel.g);
			val_b += (lv.b - pixel.b);

			target->pixels[ti].rgba.r = val_r * iarr;
			target->pixels[ti].rgba.g = val_g * iarr;
			target->pixels[ti].rgba.b = val_b * iarr;
			ti++;
		}
	}
}

void __attribute__((fastcall)) boxBlur(Bitmap32* source, Bitmap32* target, uint16_t width, uint16_t height, uint8_t radius) {
	for (uint32_t i = 0; i < source->width * source->height; i++)
		target->pixels[i].color = source->pixels[i].color;
	boxBlurH(target, source, width, height, radius);
	boxBlurT(source, target, width, height, radius);
}

uint8_t* boxesForGauss(double sigma, uint8_t n) {
	double wIdeal = sqrt((12 * sigma * sigma / n) + 1); // Ideal averaging filter width
	uint8_t wl = (uint8_t)floor(wIdeal);
	if (wl % 2 == 0) wl--;
	uint8_t wu = wl + 2;

	double mIdeal = (12 * sigma * sigma - n * wl * wl - 4 * n * wl - 3 * n) / (-4 * wl - 4);
	#define round(x) ((x)>=0?(long)((x)+0.5):(long)((x)-0.5))
	int8_t m = round(mIdeal);
	#undef round

	uint8_t* sizes = allocateFromBuffer(n);
	for (uint8_t i = 0; i < n; i++) {
		sizes[i] = i < m ? wl : wu;
	}
	return sizes;
}

Bitmap32 *gaussBlur(Bitmap32* bitmap, double radius) {
	Bitmap32* target = allocateBitmapFromBuffer(bitmap->width, bitmap->height);

	uint8_t* boxes = boxesForGauss(radius, 3);
	boxBlur(bitmap, target, bitmap->width, bitmap->height, (boxes[0] - 1) / 2);
	boxBlur(target, bitmap, bitmap->width, bitmap->height, (boxes[1] - 1) / 2);
	boxBlur(bitmap, target, bitmap->width, bitmap->height, (boxes[2] - 1) / 2);

	freeFromBuffer(3, boxes);
	return target;
}

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
