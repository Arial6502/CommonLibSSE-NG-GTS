#pragma once

#include "RE/H/HeldStateHandler.h"

namespace RE
{
	struct SprintHandler : public HeldStateHandler
	{
	public:
		inline static constexpr auto RTTI = RTTI_SprintHandler;

		~SprintHandler() override;  // 00

		// override (PlayerInputHandler)
		bool CanProcess(InputEvent* a_event) override;                                  // 01
#ifndef ENABLE_SKYRIM_AE  // 1.7.99 moves these slots, see MenuEventHandler / PlayerInputHandler
		void ProcessButton(ButtonEvent* a_event, PlayerControlsData* a_data) override;  // 04
#endif
	};
	static_assert(sizeof(SprintHandler) == 0x18);
}
