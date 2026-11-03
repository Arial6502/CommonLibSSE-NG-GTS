#pragma once

#include "RE/N/NiPoint3.h"
#include "RE/P/PlayerInputHandler.h"
#include "RE/T/TESCameraState.h"
#include "SKSE/Version.h"

namespace RE
{
	class NiNode;

	class FirstPersonState :
		public TESCameraState,     // 00
		public PlayerInputHandler  // 20
	{
	public:
		inline static constexpr auto RTTI = RTTI_FirstPersonState;
		inline static constexpr auto VTABLE = VTABLE_FirstPersonState;

		~FirstPersonState() override;  // 00

		// override (TESCameraState)
		void Begin() override;                                               // 01
		void End() override;                                                 // 02
		void Update(BSTSmartPointer<TESCameraState>& a_nextState) override;  // 03
		void GetRotation(NiQuaternion& a_rotation) override;                 // 04
		void GetTranslation(NiPoint3& a_translation) override;               // 05
		void SaveGame(BGSSaveFormBuffer* a_buf) override;                    // 06
		void LoadGame(BGSLoadFormBuffer* a_buf) override;                    // 07
		void Revert(BGSLoadFormBuffer* a_buf) override;                      // 08

		// override (PlayerInputHandler)
		bool CanProcess(InputEvent* a_event) override;                                          // 01
#ifndef ENABLE_SKYRIM_AE  // 1.7.99 moves these slots, see MenuEventHandler / PlayerInputHandler
		void ProcessButton(ButtonEvent* a_event, PlayerControlsData* a_movementData) override;  // 04
#endif

		// members
		NiPoint3      lastPosition;             // 30
		NiPoint3      lastFrameSpringVelocity;  // 3C
		NiPoint3      dampeningOffset;          // 48
		std::uint32_t pad54;                    // 54
		NiAVObject*   firstPersonCameraObj;     // 58
		NiNode*       firstPersonFOVControl;    // 60
		float         sittingRotation;          // 68
		float         unk6C;                    // 6C
		float         unk70;                    // 70
		float         currentPitchOffset;       // 74 - [-100, 100]
		float         targetPitchOffset;        // 78 - [-100, 100]

		// SE and 1.6. Starts at 0x7C.
		struct RUNTIME_DATA
		{
			float         unk7C;                // 00 (7C)
			std::uint32_t unk80;                // 04 (80)
			bool          cameraOverride;       // 08 (84)
			bool          cameraPitchOverride;  // 09 (85)
			std::uint8_t  unk86;                // 0A (86)
			std::uint8_t  unk87;                // 0B (87)
			std::uint32_t unk88;                // 0C (88)
			std::uint32_t unk8C;                // 10 (8C)
		};
		static_assert(sizeof(RUNTIME_DATA) == 0x14);

		// 1.7.99 and later: a dword is inserted at 0x7C and the rest moves +4. Starts at 0x7C.
		struct AE1799_RUNTIME_DATA
		{
			std::uint32_t unk7C;                // 00 (7C) - new
			float         unk80;                // 04 (80) - unk7C
			std::uint32_t unk84;                // 08 (84) - unk80
			bool          cameraOverride;       // 0C (88)
			bool          cameraPitchOverride;  // 0D (89)
			std::uint8_t  unk8A;                // 0E (8A) - unk86
			std::uint8_t  unk8B;                // 0F (8B) - unk87
			std::uint32_t unk8C;                // 10 (8C) - unk88
			std::uint32_t unk90;                // 14 (90) - unk8C
		};
		static_assert(sizeof(AE1799_RUNTIME_DATA) == 0x18);

		// Null from 1.7.99 on, use GetAE1799RuntimeData() there.
		[[nodiscard]] RUNTIME_DATA* GetRuntimeData() noexcept
		{
			return REL::Module::IsAtLeast(SKSE::RUNTIME_SSE_1_7_99) ? nullptr : &REL::RelocateMember<RUNTIME_DATA>(this, 0x7C);
		}

		// Null below 1.7.99, use GetRuntimeData() there.
		[[nodiscard]] AE1799_RUNTIME_DATA* GetAE1799RuntimeData() noexcept
		{
			return REL::Module::IsAtLeast(SKSE::RUNTIME_SSE_1_7_99) ? &REL::RelocateMember<AE1799_RUNTIME_DATA>(this, 0x7C) : nullptr;
		}

	private:
		std::uint8_t _padRuntimeData[sizeof(RUNTIME_DATA)];  // 7C - use GetRuntimeData() or GetAE1799RuntimeData()
	};
	static_assert(sizeof(FirstPersonState) == 0x90);
}
