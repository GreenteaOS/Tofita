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

// Based on https://forum.osdev.org/viewtopic.php?f=8&t=30052

struct Spinlock {
  private:
	// TODO is it faster to use uint8_t?
	volatile uint32_t value;

  public:
	function inline lock() {
		// TODO uint8_t tries = 255;
		// TODO yield to another process
		while (!__sync_bool_compare_and_swap(&value, 0, 1)) {
			while (value) {
				amd64::pause();
			}
		}
	}

	// TODO C++20 inline attribute
	function inline unlock() {
		__sync_lock_release(&value);
	}
};

_Static_assert(sizeof(Spinlock) == 4, "sizeof is incorrect");
