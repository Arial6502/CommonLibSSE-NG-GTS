#pragma once

#include "RE/B/BSFixedString.h"
#include "RE/B/BSInputDevice.h"
#include "RE/B/BSTArray.h"
#include "RE/B/BSTEvent.h"
#include "RE/B/BSTSingleton.h"
#include "RE/I/InputDevices.h"
#include "RE/P/PCGamepadType.h"
#include "RE/U/UserEvents.h"
#include "SKSE/Version.h"

namespace RE
{
	class UserEventEnabled;

	class ControlMap :
		public BSTSingletonSDM<ControlMap>,      // 00
		public BSTEventSource<UserEventEnabled>  // 08
	{
	public:
		using InputContextID = UserEvents::INPUT_CONTEXT_ID;
		using UEFlag = UserEvents::USER_EVENT_FLAG;

		enum : std::uint32_t
		{
			kInvalid = static_cast<std::uint8_t>(-1)
		};

		struct UserEventMapping
		{
		public:
			// members
			BSFixedString                           eventID;             // 00
			std::uint16_t                           inputKey;            // 08
			std::uint16_t                           modifier;            // 08
			std::int8_t                             indexInContext;      // 0C
			bool                                    remappable;          // 0D
			bool                                    linked;              // 0E
			stl::enumeration<UEFlag, std::uint32_t> userEventGroupFlag;  // 10
			std::uint32_t                           pad14;               // 14
		};
		static_assert(sizeof(UserEventMapping) == 0x18);

		struct InputContext
		{
		public:
			[[nodiscard]] static SKYRIM_REL_VR std::size_t GetNumDeviceMappings() noexcept
			{
#ifndef SKYRIM_CROSS_VR
				return INPUT_DEVICES::kTotal;
#else
				if SKYRIM_REL_VR_CONSTEXPR (REL::Module::IsVR()) {
					return INPUT_DEVICES::kTotal;
				} else {
					return static_cast<std::size_t>(INPUT_DEVICES::kVirtualKeyboard) + 1;
				}
#endif
			}

			// members
			BSTArray<UserEventMapping> deviceMappings[INPUT_DEVICES::kTotal];  // 00
		};
#ifdef ENABLE_SKYRIM_VR
		static_assert(sizeof(InputContext) == 0xA8);
#else
		static_assert(sizeof(InputContext) == 0x60);
#endif

		struct LinkedMapping
		{
		public:
			// members
			BSFixedString  linkedMappingName;     // 00
			InputContextID linkedMappingContext;  // 08
			INPUT_DEVICE   device;                // 0C
			InputContextID linkFromContext;       // 10
			std::uint32_t  pad14;                 // 14
			BSFixedString  linkFromName;          // 18
		};
		static_assert(sizeof(LinkedMapping) == 0x20);

		static ControlMap* GetSingleton();

		std::int8_t      AllowTextInput(bool a_allow);
		[[nodiscard]] bool AreControlsEnabled(UEFlag a_flags) const noexcept { return GetRuntimeData().enabledControls.all(a_flags); }
		std::uint32_t    GetMappedKey(std::string_view a_eventID, INPUT_DEVICE a_device, InputContextID a_context = InputContextID::kGameplay) const;
		std::string_view GetUserEventName(std::uint32_t a_buttonID, INPUT_DEVICE a_device, InputContextID a_context = InputContextID::kGameplay) const;
		[[nodiscard]] bool IsActivateControlsEnabled() const noexcept { return GetRuntimeData().enabledControls.all(UEFlag::kActivate); }
		[[nodiscard]] bool IsConsoleControlsEnabled() const noexcept { return GetRuntimeData().enabledControls.all(UEFlag::kConsole); }
		[[nodiscard]] bool IsFightingControlsEnabled() const noexcept { return GetRuntimeData().enabledControls.all(UEFlag::kFighting); }
		[[nodiscard]] bool IsLookingControlsEnabled() const noexcept { return GetRuntimeData().enabledControls.all(UEFlag::kLooking); }
		[[nodiscard]] bool IsMenuControlsEnabled() const noexcept { return GetRuntimeData().enabledControls.all(UEFlag::kMenu); }
		[[nodiscard]] bool IsMainFourControlsEnabled() const noexcept { return GetRuntimeData().enabledControls.all(UEFlag::kMainFour); }
		[[nodiscard]] bool IsMovementControlsEnabled() const noexcept { return GetRuntimeData().enabledControls.all(UEFlag::kMovement); }
		[[nodiscard]] bool IsPOVSwitchControlsEnabled() const noexcept { return GetRuntimeData().enabledControls.all(UEFlag::kPOVSwitch); }
		[[nodiscard]] bool IsSneakingControlsEnabled() const noexcept { return GetRuntimeData().enabledControls.all(UEFlag::kSneaking); }
		[[nodiscard]] bool IsVATSControlsEnabled() const noexcept { return GetRuntimeData().enabledControls.all(UEFlag::kVATS); }
		[[nodiscard]] bool IsWheelZoomControlsEnabled() const noexcept { return GetRuntimeData().enabledControls.all(UEFlag::kWheelZoom); }
		void             ToggleControls(UEFlag a_flags, bool a_enable);

		struct RUNTIME_DATA
		{
			BSTArray<LinkedMapping>                          linkedMappings;                // 00 (SE 0E8, AE 0F0)
			BSTArray<InputContextID>                         contextPriorityStack;          // 18
			stl::enumeration<UEFlag, std::uint32_t>          enabledControls;               // 30
			stl::enumeration<UEFlag, std::uint32_t>          unk11C;                        // 34 - stored controls
			std::int8_t                                      textEntryCount;                // 38
			bool                                             ignoreKeyboardMouse;           // 39
			bool                                             ignoreActivateDisabledEvents;  // 3A
			std::uint8_t                                     pad123;                        // 3B
			stl::enumeration<PC_GAMEPAD_TYPE, std::uint32_t> gamePadMapType;                // 3C
		};
		static_assert(sizeof(RUNTIME_DATA) == 0x40);

		// 1.6.1130 adds an input context, so controlMap grows by one entry and this block
		// moves from 0xE8 to 0xF0.
		[[nodiscard]] inline RUNTIME_DATA& GetRuntimeData() noexcept
		{
			return REL::RelocateMemberIfNewer<RUNTIME_DATA>(SKSE::RUNTIME_SSE_1_6_1130, this, 0xE8, 0xF0);
		}

		[[nodiscard]] inline const RUNTIME_DATA& GetRuntimeData() const noexcept
		{
			return REL::RelocateMemberIfNewer<RUNTIME_DATA>(SKSE::RUNTIME_SSE_1_6_1130, this, 0xE8, 0xF0);
		}

		// members
		InputContext* controlMap[InputContextID::kTotal];  // 060

	private:
		std::uint8_t _padRuntimeData[sizeof(RUNTIME_DATA)];  // 0E8 - use GetRuntimeData()
	};
	static_assert(sizeof(ControlMap) == 0x128);
}
