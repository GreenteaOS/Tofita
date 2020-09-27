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

namespace user32 {
struct ClassesLinkedList {
	// TODO char16_t
	const uint16_t *name = nullptr;
	wapi::WindowClass wc;
	ClassesLinkedList *next = nullptr;
};

ClassesLinkedList rootClass;

// TODO some HandleWrapper with handle type field?
constexpr uint16_t windowIsWindow = 0xA1CE;

struct Window {
	uint16_t windowIsWindow;
	uint64_t windowId;
	// TODO bool isNativeWindow;
	wapi::WindowProcedure proc;
	nj::WindowFramebuffer fb;
	uint64_t instanceId; // Random unique ID
};

constexpr uint16_t windowIsRemoteWindow = 0xA1CB;

// Window of other process
struct RemoteWindow {
	uint16_t windowIsRemoteWindow;
	uint64_t pid;
	wapi::HWnd hWnd;
	uint64_t instanceId; // Random unique ID
};
} // namespace user32
