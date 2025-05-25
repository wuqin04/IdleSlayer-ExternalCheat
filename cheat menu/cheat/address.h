#pragma once
#include <vector>


namespace address
{
	// base addressess
	namespace base
	{
		constexpr uintptr_t slayerPoint{ 0x02BB34D0 };
		constexpr uintptr_t coins{ 0x02BF0BC8 };
		constexpr uintptr_t powerUp{ 0x02BF7BD8 };
		constexpr uintptr_t jewels{ 0x02BF2388 };
		constexpr uintptr_t portalCD{ 0x02DB8CA0 };
		constexpr uintptr_t boostCD{ 0x02BE1328 };
	}
	
	// offsets
	namespace offset
	{
		std::vector<uintptr_t> slayerPoint{ 0x98, 0x110, 0xB8, 0x160 };
		std::vector<uintptr_t> coins{ 0x78, 0xB8, 0x0, 0x20 };
		std::vector<uintptr_t> powerUp{ 0x1C0, 0x388, 0x0, 0x38 };
		std::vector<uintptr_t> jewels{ 0x40, 0xB8, 0x20, 0x308 };
		std::vector<uintptr_t> portalCD{ 0x210, 0x2E0, 0x7C0, 0x468 };
		std::vector<uintptr_t> boostCD{ 0x40, 0xB8, 0x0, 0x1F8 };
	}
}

