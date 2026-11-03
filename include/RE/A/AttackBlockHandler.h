#pragma once

#include "RE/B/BSFixedString.h"
#include "RE/H/HeldStateHandler.h"
#include "SKSE/Version.h"

namespace RE
{
	struct AttackBlockHandler : public HeldStateHandler
	{
	public:
		inline static constexpr auto RTTI = RTTI_AttackBlockHandler;

		enum class AttackType : std::uint8_t
		{
			kRight = 0,
			kLeft = 1,
			kDual = 2
		};

		~AttackBlockHandler() override;  // 00

		// override (PlayerInputHandler)
		bool CanProcess(InputEvent* a_event) override;                                  // 01
#ifndef ENABLE_SKYRIM_AE  // 1.7.99 moves these slots, see MenuEventHandler / PlayerInputHandler
		void ProcessButton(ButtonEvent* a_event, PlayerControlsData* a_data) override;  // 04
		void UpdateHeldStateActive(const ButtonEvent* a_event) override;                // 05
		void SetHeldStateActive(bool a_flag) override;                                  // 06
#endif

		// SE and 1.6. Starts at 0x18.
		struct RUNTIME_DATA
		{
			std::uint32_t heldTimeMs;                  // 00 (18)
			std::uint32_t unk1C;                       // 04 (1C)
			BSFixedString controlID;                   // 08 (20)
			AttackType    attackType;                  // 10 (28)
			std::uint8_t  pad29;                       // 11 (29)
			std::uint16_t pad2A;                       // 12 (2A)
			std::uint8_t  attackCount;                 // 14 (2C)
			float         initialPowerAttackDelay;     // 18 (30)
			std::uint32_t pad34;                       // 1C (34)
			float         subsequentPowerAttackDelay;  // 20 (38)
			bool          ignore;                      // 28 (40)
			bool          unk41;                       // 29 (41)
			bool          heldLeft;                    // 2A (42)
			bool          heldRight;                   // 2B (43)
			std::uint32_t unk44;                       // 2C (44)
		};
		static_assert(sizeof(RUNTIME_DATA) == 0x30);

		// 1.7.99 and later. Eight strings are inserted before heldTimeMs, 0x34 bytes after
		// controlID and 4 after attackCount, so the object grows to 0xC0. Starts at 0x18.
		struct AE1799_RUNTIME_DATA
		{
			BSFixedString unk18[8];                    // 00 (18)
			std::uint32_t heldTimeMs;                  // 40 (58)
			std::uint32_t unk5C;                       // 44 (5C)
			BSFixedString controlID;                   // 48 (60)
			std::uint8_t  unk68[0x34];                 // 50 (68)
			AttackType    attackType;                  // 84 (9C)
			std::uint8_t  pad9D;                       // 85 (9D)
			std::uint16_t pad9E;                       // 86 (9E)
			std::uint8_t  attackCount;                 // 88 (A0)
			std::uint8_t  padA1[7];                    // 89 (A1)
			float         initialPowerAttackDelay;     // 90 (A8)
			std::uint32_t padAC;                       // 94 (AC)
			float         subsequentPowerAttackDelay;  // 98 (B0)
			std::uint32_t padB4;                       // 9C (B4)
			bool          ignore;                      // A0 (B8)
			bool          unkB9;                       // A1 (B9)
			bool          heldLeft;                    // A2 (BA)
			bool          heldRight;                   // A3 (BB)
			std::uint32_t unkBC;                       // A4 (BC)
		};
		static_assert(sizeof(AE1799_RUNTIME_DATA) == 0xA8);

		// Null from 1.7.99 on, use GetAE1799RuntimeData() there.
		[[nodiscard]] RUNTIME_DATA* GetRuntimeData() noexcept
		{
			return REL::Module::IsAtLeast(SKSE::RUNTIME_SSE_1_7_99) ? nullptr : &REL::RelocateMember<RUNTIME_DATA>(this, 0x18);
		}

		// Null below 1.7.99, use GetRuntimeData() there.
		[[nodiscard]] AE1799_RUNTIME_DATA* GetAE1799RuntimeData() noexcept
		{
			return REL::Module::IsAtLeast(SKSE::RUNTIME_SSE_1_7_99) ? &REL::RelocateMember<AE1799_RUNTIME_DATA>(this, 0x18) : nullptr;
		}

	private:
		std::uint8_t _padRuntimeData[sizeof(RUNTIME_DATA)];  // 18 - use GetRuntimeData() or GetAE1799RuntimeData()
	};
	static_assert(sizeof(AttackBlockHandler) == 0x48);
}
