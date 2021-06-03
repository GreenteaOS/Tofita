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

extern "C++" namespace shortcut {
	constexpr uint32_t magic = 0x0000004C;
	constexpr uint8_t cls[16] = {0, 0x02, 0x14, 0x01, 0, 0, 0, 0, 0xC0, 0, 0, 0, 0, 0, 0, 0x46};

	__attribute__((packed)) struct LnkFlags {
		uint8_t shellItemIdListIsPresent : 1;
		uint8_t pointsToFileOrDirectory : 1;
		uint8_t hasDescription : 1;
		uint8_t hasRelativePath : 1;
		uint8_t hasWorkingDirectory : 1;
		uint8_t hasCommandLineArguments : 1;
		uint8_t hasCustomIcon : 1;
		uint32_t padding : 25;
	};

	_Static_assert(sizeof(LnkFlags) == 4, "bad sizeof");

	 struct __attribute__((packed)) LnkAttributes {
		uint8_t readOnly : 1;
		uint8_t hidden : 1;
		uint8_t systemFile : 1;
		uint8_t volumeLabel : 1;
		uint8_t directory : 1;
		uint8_t targetArchiveChanged : 1;
		uint8_t encrypted : 1;
		uint8_t none : 1;
		uint8_t temporary : 1;
		uint8_t sparseFile : 1;
		uint8_t hasReparsePointData : 1;
		uint8_t compressed : 1;
		uint8_t offline : 1;
		uint32_t padding : 19;
	};

	_Static_assert(sizeof(LnkAttributes) == 4, "bad sizeof");

#pragma pack(1)
	struct __attribute__((packed)) Lnk {
		uint32_t magucka;
		uint8_t clus[16];
		LnkFlags flugs;
		LnkAttributes atts;
		uint64_t timeOfCreation;
		uint64_t timeOfModification;
		uint64_t timeOfAccess;
		uint32_t lengthOfTargetFileBytesOfLeastBits;
		uint32_t iconIndex;
		uint32_t showWund;
		uint16_t hotKey;
		uint8_t padding[10];
	};
#pragma pack()

	_Static_assert(sizeof(Lnk) == 0x0000004C, "bad sizeof");
}
