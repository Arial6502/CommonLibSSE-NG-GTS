#pragma once

#include "RE/M/MapInputHandler.h"

namespace RE
{
	class MapMoveHandler : public MapInputHandler
	{
	public:
		inline static constexpr auto RTTI = RTTI_MapMoveHandler;

		~MapMoveHandler() override;  // 00

		// override (MapInputHandler)
		bool CanProcess(InputEvent* a_event) override;              // 01
#ifndef ENABLE_SKYRIM_AE  // 1.7.99 moves these slots, see MenuEventHandler / PlayerInputHandler
		bool ProcessThumbstick(ThumbstickEvent* a_event) override;  // 03
#endif

		// members
		std::uint64_t unk18;  // 18
	};
	static_assert(sizeof(MapMoveHandler) == 0x20);
}
