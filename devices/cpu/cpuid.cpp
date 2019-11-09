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

struct CPUID {
	char vendorID[13] = {0};
	char brandName[48] = {0};
	uint32_t data = 0;
	uint32_t extras = 0;
};

const uint32_t cpuidExtendedLevels = 0x80000000;

CPUID getCPUID() {
	CPUID result;
	amd64::cpuid(0, 0, &result.data, (uint32_t *)(&result.vendorID[0]),
				 (uint32_t *)(&result.vendorID[8]),
				 (uint32_t *)(&result.vendorID[4]));

	amd64::cpuid(cpuidExtendedLevels, 0, &result.extras);

	if (result.extras >= cpuidExtendedLevels + 4) {
		amd64::cpuid(cpuidExtendedLevels + 2, 0,
					 (uint32_t *)(&result.brandName[0]),
					 (uint32_t *)(&result.brandName[4]),
					 (uint32_t *)(&result.brandName[8]),
					 (uint32_t *)(&result.brandName[12]));
		amd64::cpuid(cpuidExtendedLevels + 3, 0,
					 (uint32_t *)(&result.brandName[16]),
					 (uint32_t *)(&result.brandName[20]),
					 (uint32_t *)(&result.brandName[24]),
					 (uint32_t *)(&result.brandName[28]));
		amd64::cpuid(cpuidExtendedLevels + 4, 0,
					 (uint32_t *)(&result.brandName[32]),
					 (uint32_t *)(&result.brandName[36]),
					 (uint32_t *)(&result.brandName[40]),
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
