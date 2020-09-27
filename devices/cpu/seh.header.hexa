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

volatile bool sehProbe = false;

// This is a fascinating way to not allow compiler to optimize return value
extern volatile bool (*volatile const probeForReadOkay)(volatile uint64_t at, volatile const uint64_t bytes);
extern volatile bool (*volatile const probeForWriteOkay)(volatile const uint64_t at,
														 volatile const uint64_t bytes);

// TODO template wrappers
/*
<T>
class UserRead {
	const *T t;
	probeForReadOkay return t;
}

class UserReadWrite {
	*T t;
	probeForWriteOkay return t;
}

*/
