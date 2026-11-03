#pragma once

#include "RE/P/PlayerInputHandler.h"

namespace RE
{
	class HeldStateHandler : public PlayerInputHandler
	{
	public:
		inline static constexpr auto RTTI = RTTI_HeldStateHandler;

		~HeldStateHandler() override;  // 00

#ifndef ENABLE_SKYRIM_AE
		// add
		virtual void UpdateHeldStateActive(const ButtonEvent* a_event);  // 05
		virtual void SetHeldStateActive(bool a_flag);                    // 06 - { heldStateActive = a_flag; }
#else
		// 05 and 06, 07 and 08 from 1.7.99 (see PlayerInputHandler).
		void UpdateHeldStateActive(const ButtonEvent* a_event) { REL::RelocateVirtual<void(HeldStateHandler*, const ButtonEvent*)>(Slot(0x05), 0x05, this, a_event); }
		void SetHeldStateActive(bool a_flag) { REL::RelocateVirtual<void(HeldStateHandler*, bool)>(Slot(0x06), 0x06, this, a_flag); }
#endif

		// members
		bool          heldStateActive;      // 10
		bool          triggerReleaseEvent;  // 11
		std::uint16_t pad12;                // 12
		std::uint32_t pad14;                // 14
	};
	static_assert(sizeof(HeldStateHandler) == 0x18);
}
