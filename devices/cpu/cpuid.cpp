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

struct CPUID {
	char8_t vendorID[13] = {0};
	char8_t brandName[48] = {0};
	uint32_t data = 0;
	uint32_t extras = 0;
};

constexpr uint32_t cpuidExtendedLevels = 0x80000000;

CPUID getCPUID() {
	CPUID result;
	uint32_t stub = 0;
	amd64::cpuid(0, 0, &result.data, (uint32_t *)(&result.vendorID[0]), (uint32_t *)(&result.vendorID[8]),
				 (uint32_t *)(&result.vendorID[4]));

	amd64::cpuid(cpuidExtendedLevels, 0, &result.extras, &stub, &stub, &stub);

	if (result.extras >= cpuidExtendedLevels + 4) {
		amd64::cpuid(cpuidExtendedLevels + 2, 0, (uint32_t *)(&result.brandName[0]),
					 (uint32_t *)(&result.brandName[4]), (uint32_t *)(&result.brandName[8]),
					 (uint32_t *)(&result.brandName[12]));
		amd64::cpuid(cpuidExtendedLevels + 3, 0, (uint32_t *)(&result.brandName[16]),
					 (uint32_t *)(&result.brandName[20]), (uint32_t *)(&result.brandName[24]),
					 (uint32_t *)(&result.brandName[28]));
		amd64::cpuid(cpuidExtendedLevels + 4, 0, (uint32_t *)(&result.brandName[32]),
					 (uint32_t *)(&result.brandName[36]), (uint32_t *)(&result.brandName[40]),
					 (uint32_t *)(&result.brandName[44]));
	}

	var i = 0;
	while (i++ < sizeof(result.brandName))
		if (result.brandName[i] == '\r')
			result.brandName[i] = 0;
		else if (result.brandName[i] == '\n')
			result.brandName[i] = 0;

	i = 0;
	while (i++ < sizeof(result.vendorID))
		if (result.vendorID[i] == '\r')
			result.vendorID[i] = 0;
		else if (result.vendorID[i] == '\n')
			result.vendorID[i] = 0;

	return result;
}
