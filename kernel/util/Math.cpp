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
