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

extern "C++" class Math {
  public:
	static int32_t round(double value) {
		return ((value) >= 0 ? (int32_t)((value) + 0.5) : (int32_t)((value)-0.5));
	}

	static float sqrt(float number) {
		int32_t i;
		float x2, y;
		const float threehalfs = 1.5F;

		x2 = number * 0.5F;
		y = number;
		i = *(int32_t *)&y;		   // floating point bit level hacking [sic]
		i = 0x5f3759df - (i >> 1); // Newton's approximation
		y = *(float *)&i;
		y = y * (threehalfs - (x2 * y * y)); // 1st iteration
		y = y * (threehalfs - (x2 * y * y)); // 2nd iteration
		y = y * (threehalfs - (x2 * y * y)); // 3rd iteration

		return 1.0f / y;
	}

	static int32_t min(int32_t a, int32_t b) {
		return a < b ? a : b;
	}

	static double min(double a, double b) {
		return a < b ? a : b;
	}

	static int32_t max(int32_t a, int32_t b) {
		return a > b ? a : b;
	}

	static double max(double a, double b) {
		return a > b ? a : b;
	}
};
