#pragma once

#include "REL/Relocation.h"
#include "SKSE/Version.h"

namespace RE
{
	class ButtonEvent;
	class InputEvent;
	class MouseMoveEvent;
	class PlayerControlsData;
	class ThumbstickEvent;

	class PlayerInputHandler
	{
	public:
		inline static constexpr auto RTTI = RTTI_PlayerInputHandler;

		virtual ~PlayerInputHandler();  // 00

		virtual bool CanProcess(InputEvent* a_event) = 0;  // 01

#ifndef ENABLE_SKYRIM_AE
		virtual void ProcessThumbstick(ThumbstickEvent* a_event, PlayerControlsData* a_data);  // 02 - { return; }
		virtual void ProcessMouseMove(MouseMoveEvent* a_event, PlayerControlsData* a_data);    // 03 - { return; }
		virtual void ProcessButton(ButtonEvent* a_event, PlayerControlsData* a_data);          // 04 - { return; }
#else
		// 1.7.99 inserts ProcessMotionGesture (02) and ProcessSixaxis (03) after CanProcess,
		// so these move from 02-04 to 04-06. Builds with AE call them through the runtime slot
		// and cannot override them.
		void ProcessThumbstick(ThumbstickEvent* a_event, PlayerControlsData* a_data) { REL::RelocateVirtual<void(PlayerInputHandler*, ThumbstickEvent*, PlayerControlsData*)>(Slot(0x02), 0x02, this, a_event, a_data); }
		void ProcessMouseMove(MouseMoveEvent* a_event, PlayerControlsData* a_data) { REL::RelocateVirtual<void(PlayerInputHandler*, MouseMoveEvent*, PlayerControlsData*)>(Slot(0x03), 0x03, this, a_event, a_data); }
		void ProcessButton(ButtonEvent* a_event, PlayerControlsData* a_data) { REL::RelocateVirtual<void(PlayerInputHandler*, ButtonEvent*, PlayerControlsData*)>(Slot(0x04), 0x04, this, a_event, a_data); }

		[[nodiscard]] static std::ptrdiff_t Slot(std::ptrdiff_t a_index) noexcept
		{
			return REL::Module::IsAtLeast(SKSE::RUNTIME_SSE_1_7_99) ? a_index + 2 : a_index;
		}
#endif

		[[nodiscard]] bool IsInputEventHandlingEnabled() const;
		void               SetInputEventHandlingEnabled(bool a_enabled);

		// members
		bool          inputEventHandlingEnabled;  // 08
		std::uint8_t  pad09;                      // 09
		std::uint16_t pad0A;                      // 0A
		std::uint32_t pad0C;                      // 0C
	};
	static_assert(sizeof(PlayerInputHandler) == 0x10);
}
