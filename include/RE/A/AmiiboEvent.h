#pragma once

#include "RE/I/IDEvent.h"

namespace RE
{
#ifdef ENABLE_SKYRIM_AE
#	pragma pack(push, 4)
	// New in 1.7.99. INPUT_EVENT_TYPE::kAmiibo.
	class AmiiboEvent : public IDEvent
	{
	public:
		inline static constexpr auto RTTI = RTTI_AmiiboEvent;
		inline static constexpr auto VTABLE = VTABLE_AmiiboEvent;

		~AmiiboEvent() override;  // 00

		// members
		std::uint32_t unk28;  // 28
		std::uint64_t unk2C;  // 2C
		std::uint16_t unk34;  // 34
		std::uint16_t unk36;  // 36
		std::uint32_t unk38;  // 38
		std::uint32_t unk3C;  // 3C
	};
#	pragma pack(pop)
	static_assert(sizeof(AmiiboEvent) == 0x40);
#endif
}
