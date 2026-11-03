#pragma once

#include "RE/B/BSIntrusiveRefCounted.h"
#include "REL/Relocation.h"
#include "SKSE/Version.h"

namespace RE
{
	class ButtonEvent;
	class InputEvent;
	class KinectEvent;
	class MotionGestureEvent;
	class MouseMoveEvent;
	class SixaxisEvent;
	class ThumbstickEvent;

	class MenuEventHandler : public BSIntrusiveRefCounted
	{
	public:
		inline static constexpr auto RTTI = RTTI_MenuEventHandler;

		MenuEventHandler() = default;
		virtual ~MenuEventHandler() = default;  // 00

		virtual bool CanProcess(InputEvent* a_event) = 0;  // 01

#ifndef ENABLE_SKYRIM_AE
		virtual bool ProcessKinect(KinectEvent* a_event);          // 02 - { return false; }
		virtual bool ProcessThumbstick(ThumbstickEvent* a_event);  // 03 - { return false; }
		virtual bool ProcessMouseMove(MouseMoveEvent* a_event);    // 04 - { return false; }
		virtual bool ProcessButton(ButtonEvent* a_event);          // 05 - { return false; }
#else
		// 1.7.99 inserts ProcessMotionGesture (02) and ProcessSixaxis (03) after CanProcess,
		// so these move from 02-05 to 04-07. A compile time vtable cannot match both
		// layouts, so builds with AE call them through the runtime slot and cannot override
		// them.
		bool ProcessKinect(KinectEvent* a_event) { return REL::RelocateVirtual<bool(MenuEventHandler*, KinectEvent*)>(Slot(0x02), 0x02, this, a_event); }
		bool ProcessThumbstick(ThumbstickEvent* a_event) { return REL::RelocateVirtual<bool(MenuEventHandler*, ThumbstickEvent*)>(Slot(0x03), 0x03, this, a_event); }
		bool ProcessMouseMove(MouseMoveEvent* a_event) { return REL::RelocateVirtual<bool(MenuEventHandler*, MouseMoveEvent*)>(Slot(0x04), 0x04, this, a_event); }
		bool ProcessButton(ButtonEvent* a_event) { return REL::RelocateVirtual<bool(MenuEventHandler*, ButtonEvent*)>(Slot(0x05), 0x05, this, a_event); }

		// The slots 1.7.99 inserts (02, 03). They do nothing and return false below 1.7.99.
		bool ProcessMotionGesture(MotionGestureEvent* a_event)
		{
			return REL::Module::IsAtLeast(SKSE::RUNTIME_SSE_1_7_99) && REL::RelocateVirtual<bool(MenuEventHandler*, MotionGestureEvent*)>(0x02, 0x02, this, a_event);
		}
		bool ProcessSixaxis(SixaxisEvent* a_event)
		{
			return REL::Module::IsAtLeast(SKSE::RUNTIME_SSE_1_7_99) && REL::RelocateVirtual<bool(MenuEventHandler*, SixaxisEvent*)>(0x03, 0x03, this, a_event);
		}

		[[nodiscard]] static std::ptrdiff_t Slot(std::ptrdiff_t a_index) noexcept
		{
			return REL::Module::IsAtLeast(SKSE::RUNTIME_SSE_1_7_99) ? a_index + 2 : a_index;
		}
#endif

		// members
		bool          registered;  // 0C
		std::uint8_t  unk0D;       // 0D
		std::uint16_t pad0E;       // 0E
	};
	static_assert(sizeof(MenuEventHandler) == 0x10);
}
