#pragma once

#include "RE/P/PlayerInputHandler.h"

namespace RE
{
	struct LookHandler : public PlayerInputHandler
	{
	public:
		inline static constexpr auto RTTI = RTTI_LookHandler;

		~LookHandler() override;  // 00

		// override (PlayerInputHandler)
		bool CanProcess(InputEvent* a_event) override;                                          // 01
#ifndef ENABLE_SKYRIM_AE  // 1.7.99 moves these slots, see MenuEventHandler / PlayerInputHandler
		void ProcessThumbstick(ThumbstickEvent* a_event, PlayerControlsData* a_data) override;  // 02
		void ProcessMouseMove(MouseMoveEvent* a_event, PlayerControlsData* a_data) override;    // 03
#endif
	};
	static_assert(sizeof(LookHandler) == 0x10);
}
