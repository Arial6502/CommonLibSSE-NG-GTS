#pragma once

#include "RE/B/BSSoundHandle.h"
#include "RE/B/BSTEvent.h"
#include "RE/I/IMenu.h"
#include "RE/M/MenuEventHandler.h"
#include "RE/N/NiMatrix3.h"
#include "SKSE/Version.h"

namespace RE
{
	class MenuOpenCloseEvent;
	class NiAVObject;
	class NiControllerManager;
	class NiControllerSequence;
	class TESObjectREFR;

	class ModelData
	{
	public:
		std::uint64_t unk00;   // 00
		std::uint64_t unk08;   // 08
		std::uint64_t unk10;   // 10
		std::uint64_t unk18;   // 00
		std::uint64_t unk20;   // 00
		NiAVObject*   object;  // 28
	};

	// menuDepth = 3
	// flags = kPausesGame | kDisablePauseMenu | kRequiresUpdate
	// context = kLockpicking
	class LockpickingMenu :
		public IMenu,  // 00
#ifndef SKYRIM_CROSS_VR
		public MenuEventHandler,                 // 30
		public BSTEventSink<MenuOpenCloseEvent>  // 40
#else
		public MenuEventHandler  // 30
#endif
	{
	public:
		inline static constexpr auto      RTTI = RTTI_LockpickingMenu;
		constexpr static std::string_view MENU_NAME = "Lockpicking Menu";

		struct RUNTIME_DATA
		{
#define RUNTIME_DATA_CONTENT                            \
	ModelData*            lockpickShiv;        /* 00 */ \
	ModelData*            lockpick;            /* 08 */ \
	NiMatrix3             unk058;              /* 10 */ \
	NiPoint3              lockRotCenter;       /* 34 */ \
	NiControllerManager*  shivController;      /* 40 */ \
	NiControllerSequence* shivIntro;           /* 48 */ \
	NiControllerSequence* shivRotate;          /* 50 */ \
	NiControllerManager*  pickController;      /* 58 */ \
	NiControllerSequence* pickIntro;           /* 60 */ \
	NiControllerSequence* pickDamage;          /* 68 */ \
	NiControllerSequence* pickBreak;           /* 70 */ \
	NiControllerSequence* currentPickSequence; /* 78 */ \
	float                 pickKeyTime;         /* 80 */ \
	std::uint32_t         unk0CC;              /* 84 */ \
	NiControllerSequence* currentShivSequence; /* 88 */ \
	float                 shivKeyTime;         /* 90 */ \
	std::uint32_t         unk0DC;              /* 94 */ \
	float                 currentAngle;        /* 98 */ \
	float                 lockAngle;           /* 9C */

			RUNTIME_DATA_CONTENT
		};
		static_assert(sizeof(RUNTIME_DATA) == 0xA0);

		// 1.7.99 inserts 0x14 bytes after lockAngle, so this block moves from 0xE8 to 0xFC.
		struct TAIL_RUNTIME_DATA
		{
			std::uint32_t unk0E8;  // 00
			BSSoundHandle unk0EC;  // 04
			std::uint32_t unk0F8;  // 10
			std::uint32_t unk0FC;  // 14
			std::uint32_t unk100;  // 18
			std::uint32_t unk104;  // 1C
			std::uint16_t unk108;  // 20
			std::uint8_t  unk10A;  // 22
			std::uint8_t  unk10B;  // 23
			std::uint8_t  unk10C;  // 24
			std::uint8_t  unk10D;  // 25
			std::uint8_t  unk10E;  // 26
			std::uint8_t  pad10F;  // 27
		};
		static_assert(sizeof(TAIL_RUNTIME_DATA) == 0x28);

		~LockpickingMenu() override;  // 00

		// override (IMenu)
		UI_MESSAGE_RESULTS ProcessMessage(UIMessage& a_message) override;  // 04

#ifndef SKYRIM_CROSS_VR
		// override (MenuEventHandler)
		bool CanProcess(InputEvent* a_event) override;              // 01
#ifndef ENABLE_SKYRIM_AE  // 1.7.99 moves these slots, see MenuEventHandler / PlayerInputHandler
		bool ProcessThumbstick(ThumbstickEvent* a_event) override;  // 03
		bool ProcessMouseMove(MouseMoveEvent* a_event) override;    // 04
		bool ProcessButton(ButtonEvent* a_event) override;          // 05
#endif

		// override (BSTEventSink<MenuOpenCloseEvent>)
		BSEventNotifyControl ProcessEvent(const MenuOpenCloseEvent* a_event, BSTEventSource<MenuOpenCloseEvent>* a_eventSource) override;  // 01
#endif

		[[nodiscard]] static TESObjectREFR* GetTargetReference();

		[[nodiscard]] MenuEventHandler* AsMenuEventHandler() noexcept
		{
			return &REL::RelocateMember<MenuEventHandler>(this, 0x30, 0x40);
		}

		[[nodiscard]] const MenuEventHandler* AsMenuEventHandler() const noexcept
		{
			return const_cast<LockpickingMenu*>(this)->AsMenuEventHandler();
		}

		[[nodiscard]] BSTEventSink<MenuOpenCloseEvent>* AsMenuOpenCloseEventSink() noexcept
		{
			return &REL::RelocateMember<BSTEventSink<MenuOpenCloseEvent>>(this, 0x40, 0x50);
		}

		[[nodiscard]] const BSTEventSink<MenuOpenCloseEvent>* AsMenuOpenCloseEventSink() const noexcept
		{
			return const_cast<LockpickingMenu*>(this)->AsMenuOpenCloseEventSink();
		}

		[[nodiscard]] inline RUNTIME_DATA& GetRuntimeData() noexcept
		{
			return REL::RelocateMember<RUNTIME_DATA>(this, 0x48, 0x58);
		}

		[[nodiscard]] inline const RUNTIME_DATA& GetRuntimeData() const noexcept
		{
			return REL::RelocateMember<RUNTIME_DATA>(this, 0x48, 0x58);
		}

		[[nodiscard]] inline TAIL_RUNTIME_DATA& GetTailRuntimeData() noexcept
		{
			if SKYRIM_REL_VR_CONSTEXPR (REL::Module::IsVR()) {
				return REL::RelocateMember<TAIL_RUNTIME_DATA>(this, 0xF8);
			}
			return REL::RelocateMemberIfNewer<TAIL_RUNTIME_DATA>(SKSE::RUNTIME_SSE_1_7_99, this, 0xE8, 0xFC);
		}

		[[nodiscard]] inline const TAIL_RUNTIME_DATA& GetTailRuntimeData() const noexcept
		{
			return const_cast<LockpickingMenu*>(this)->GetTailRuntimeData();
		}

		// members
#ifndef SKYRIM_CROSS_VR
		RUNTIME_DATA_CONTENT  // 48, 58

	private:
		std::uint8_t _padTailRuntimeData[sizeof(TAIL_RUNTIME_DATA)];  // E8 - use GetTailRuntimeData()
#endif
	};
#ifndef ENABLE_SKYRIM_VR
	static_assert(sizeof(LockpickingMenu) == 0x110);
#elif !defined(ENABLE_SKYRIM_AE) && !defined(ENABLE_SKYRIM_SE)
	static_assert(sizeof(LockpickingMenu) == 0x120);
#endif
}
#undef RUNTIME_DATA_CONTENT
