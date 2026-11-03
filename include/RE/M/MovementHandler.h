#pragma once

#include "RE/P/PlayerInputHandler.h"

namespace RE
{
	struct MovementHandler : public PlayerInputHandler
	{
	public:
		inline static constexpr auto RTTI = RTTI_MovementHandler;

		~MovementHandler() override;  // 00

		// override (PlayerInputHandler)
		bool CanProcess(InputEvent* a_event) override;                                          // 01
#ifndef ENABLE_SKYRIM_AE  // 1.7.99 moves these slots, see MenuEventHandler / PlayerInputHandler
		void ProcessThumbstick(ThumbstickEvent* a_event, PlayerControlsData* a_data) override;  // 02
		void ProcessButton(ButtonEvent* a_event, PlayerControlsData* a_data) override;          // 04
#endif
	};
	static_assert(sizeof(MovementHandler) == 0x10);
}