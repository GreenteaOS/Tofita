// The Tofita Kernel
// Copyright (C) 2021  Oleg Petrenko
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

#pragma once

// Used to make payloads same size in 32/64 bit
extern "C++" template <typename T> struct Ref {
private:
	uint64_t userMemory;

public:
    operator uint64_t() const { return userMemory; }
    operator T*() const { return (T*)userMemory; }
    Ref() = default;
    Ref(T* from) {
    	userMemory = (uint64_t)from;
    }
};

_Static_assert(sizeof(Ref<uint8_t>) == 8, "bad sizeof");

// Non-pointer 64-bit values, like HWND
extern "C++" template <typename T> struct Hold {
private:
	uint64_t userValue;

public:
    operator uint64_t() const { return userValue; }
    operator T() const { return (T)userValue; }
    Hold() = default;
    Hold(T from) {
    	userValue = (uint64_t)from;
    }
};

_Static_assert(sizeof(Hold<uint8_t>) == 8, "bad sizeof");
