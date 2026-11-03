#pragma once

#include "RE/M/MapInputHandler.h"

namespace RE
{
	class MapZoomHandler : public MapInputHandler
	{
	public:
		inline static constexpr auto RTTI = RTTI_MapZoomHandler;

		~MapZoomHandler() override;  // 00

		// override (MapInputHandler)
		bool CanProcess(InputEvent* a_event) override;      // 01
#ifndef ENABLE_SKYRIM_AE  // 1.7.99 moves these slots, see MenuEventHandler / PlayerInputHandler
		bool ProcessButton(ButtonEvent* a_event) override;  // 05
#endif
	};
	static_assert(sizeof(MapZoomHandler) == 0x18);
}
