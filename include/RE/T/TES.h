#pragma once

#include "RE/B/BSTEvent.h"
#include "RE/B/BSTList.h"
#include "RE/B/BSTTuple.h"
#include "RE/I/ICellAttachDetachEventSource.h"
#include "RE/M/MaterialIDs.h"
#include "RE/N/NiPoint3.h"
#include "RE/N/NiSmartPointer.h"
#include "SKSE/Version.h"

namespace RE
{
	namespace BSResource
	{
		struct ArchiveStreamOpenedEvent;
	}

	class BSSystemEvent;
	class BSTempNodeManager;
	class GridCellArray;
	class ImageSpaceModifierInstance;
	class NavMeshInfoMap;
	class NiAVObject;
	class NiNode;
	class Sky;
	class TESLandTexture;
	class TESNPC;
	class TESObjectCELL;
	class TESObjectREFR;
	class TESWorldSpace;

	struct bhkPickData;
	struct PositionPlayerEvent;

	class TES :
		public ICellAttachDetachEventSource,                        // 000
		public BSTEventSink<BSResource::ArchiveStreamOpenedEvent>,  // 060
		public BSTEventSink<PositionPlayerEvent>                    // 068
	{
	public:
		inline static constexpr auto RTTI = RTTI_TES;

		class SystemEventAdapter : public BSTEventSink<BSSystemEvent>
		{
		public:
			inline static constexpr auto RTTI = RTTI_TES;

			~SystemEventAdapter() override;  // 00

			// override (BSTEventSink<BSSystemEvent>)
			BSEventNotifyControl ProcessEvent(const BSSystemEvent* a_event, BSTEventSource<BSSystemEvent>* a_eventSource) override;  // 01

			// members
			std::uint64_t unk08;  // 08
		};
		static_assert(sizeof(SystemEventAdapter) == 0x10);

		~TES() override;  // 00

		// override (BSTEventSink<BSResource::ArchiveStreamOpenedEvent>)
		BSEventNotifyControl ProcessEvent(const BSResource::ArchiveStreamOpenedEvent* a_event, BSTEventSource<BSResource::ArchiveStreamOpenedEvent>* a_eventSource) override;  // 01 - { return BSEventNotifyControl::kContinue; }

		// override (BSTEventSink<PositionPlayerEvent>)
		BSEventNotifyControl ProcessEvent(const PositionPlayerEvent* a_event, BSTEventSource<PositionPlayerEvent>* a_eventSource) override;  // 01

		static TES* GetSingleton();

		void ForEachCell(std::function<void(TESObjectCELL*)> a_callback) const;
		void ForEachCellInRange(TESObjectREFR* a_origin, float a_radius, std::function<void(TESObjectCELL*)> a_callback) const;
		void ForEachReferenceInRange(TESObjectREFR* a_origin, float a_radius, const std::function<BSContainer::ForEachResult(TESObjectREFR* a_ref)>& a_callback) const;
		void ForEachReference(const std::function<BSContainer::ForEachResult(TESObjectREFR* a_ref)>& a_callback) const;

		// SE, and AE before 1.6.1130.
		struct RUNTIME_DATA
		{
			std::uint8_t  unk128;           // 00 (128)
			bool          showLANDborders;  // 01 (129)
			std::uint8_t  unk12A;           // 02 (12A)
			std::uint8_t  unk12B;           // 03 (12B)
			std::uint8_t  unk12C;           // 04 (12C)
			std::uint8_t  unk12D;           // 05 (12D)
			std::uint8_t  unk12E;           // 06 (12E)
			std::uint8_t  unk12F;           // 07 (12F)
			std::uint16_t unk130;           // 08 (130)
			std::uint8_t  unk132;           // 0A (132)
			std::uint8_t  unk133;           // 0B (133)
			float         unk134;           // 0C (134)
			float         unk138;           // 10 (138)
			std::uint32_t pad13C;           // 14 (13C)
		};
		static_assert(sizeof(RUNTIME_DATA) == 0x18);

		// 1.6.1130 and later. showLANDborders became a dword mode and a dword follows it,
		// so the SE bytes from 0x12A move +0xA and the two floats +0xC. Fields are named by
		// their AE offset, with the SE field they replace.
		struct AE_RUNTIME_DATA
		{
			std::uint32_t unk128;          // 00 (128)
			std::uint32_t landBorderMode;  // 04 (12C) - showLANDborders
			std::uint32_t unk130;          // 08 (130) - new
			std::uint8_t  unk134;          // 0C (134) - unk12A
			std::uint8_t  unk135;          // 0D (135) - unk12B
			std::uint8_t  unk136;          // 0E (136) - unk12C
			std::uint8_t  unk137;          // 0F (137) - unk12D
			std::uint8_t  unk138;          // 10 (138) - unk12E
			std::uint8_t  unk139;          // 11 (139) - unk12F
			std::uint16_t unk13A;          // 12 (13A) - unk130
			std::uint8_t  unk13C;          // 14 (13C) - unk132
			std::uint8_t  unk13D;          // 15 (13D) - unk133
			std::uint16_t pad13E;          // 16 (13E)
			float         unk140;          // 18 (140) - unk134
			float         unk144;          // 1C (144) - unk138
		};
		static_assert(sizeof(AE_RUNTIME_DATA) == 0x20);

		// SE offsets. Every member here is +8 from 1.6.1130.
		struct RUNTIME_DATA2
		{
			TESWorldSpace*                                      worldSpace;                 // 140
			BSSimpleList<BSTTuple<TESNPC*, std::uint16_t>*>     deadCount;                  // 148
			void*                                               unk158;                     // 158 - smart ptr
			void*                                               unk160;                     // 160 - smart ptr
			void*                                               unk168;                     // 168 - smart ptr
			void*                                               unk170;                     // 170 - smart ptr
			std::uint64_t                                       unk178;                     // 178
			std::uint64_t                                       unk180;                     // 180
			std::uint64_t                                       unk188;                     // 188
			std::uint64_t                                       unk190;                     // 190
			std::uint64_t                                       unk198;                     // 198
			std::uint64_t                                       unk1A0;                     // 1A0
			std::uint64_t                                       unk1A8;                     // 1A8
			std::uint64_t                                       unk1B0;                     // 1B0
			std::uint64_t                                       unk1B8;                     // 1B8
			std::uint64_t                                       unk1C0;                     // 1C0
			std::uint64_t                                       unk1C8;                     // 1C8
			std::uint64_t                                       unk1D0;                     // 1D0
			std::uint64_t                                       unk1D8;                     // 1D8
			std::uint64_t                                       unk1E0;                     // 1E0
			std::uint64_t                                       unk1E8;                     // 1E8
			std::uint64_t                                       unk1F0;                     // 1F0
			std::uint64_t                                       unk1F8;                     // 1F8
			std::uint64_t                                       unk200;                     // 200
			std::uint64_t                                       unk208;                     // 208
			std::uint64_t                                       unk210;                     // 210
			std::uint64_t                                       unk218;                     // 218
			std::uint64_t                                       unk220;                     // 220
			std::uint64_t                                       unk228;                     // 228
			std::uint64_t                                       unk230;                     // 230
			std::uint64_t                                       unk238;                     // 238
			std::uint64_t                                       unk240;                     // 240
			std::uint64_t                                       unk248;                     // 248
			std::uint64_t                                       unk250;                     // 250
			std::uint64_t                                       unk258;                     // 258
			std::uint64_t                                       unk260;                     // 260
			std::uint64_t                                       unk268;                     // 268
			std::uint64_t                                       unk270;                     // 270
			std::uint64_t                                       unk278;                     // 278
			std::uint64_t                                       unk280;                     // 280
			std::uint64_t                                       unk288;                     // 288
			SystemEventAdapter                                  unk290;                     // 290
			std::uint64_t                                       unk2A0;                     // 2A0
			NavMeshInfoMap*                                     unk2A8;                     // 2A8
			std::uint64_t                                       unk2B0;                     // 2B0
		};
		static_assert(sizeof(RUNTIME_DATA2) == 0x178);

		// Null from 1.6.1130 on, use GetAERuntimeData() there.
		[[nodiscard]] inline RUNTIME_DATA* GetRuntimeData() noexcept
		{
			return REL::Module::IsAtLeast(SKSE::RUNTIME_SSE_1_6_1130) ? nullptr : &REL::RelocateMember<RUNTIME_DATA>(this, 0x128);
		}

		// Null below 1.6.1130, use GetRuntimeData() there.
		[[nodiscard]] inline AE_RUNTIME_DATA* GetAERuntimeData() noexcept
		{
			return REL::Module::IsAtLeast(SKSE::RUNTIME_SSE_1_6_1130) ? &REL::RelocateMember<AE_RUNTIME_DATA>(this, 0x128) : nullptr;
		}

		[[nodiscard]] inline RUNTIME_DATA2& GetRuntimeData2() noexcept
		{
			return REL::RelocateMemberIfNewer<RUNTIME_DATA2>(SKSE::RUNTIME_SSE_1_6_1130, this, 0x140, 0x148);
		}

		[[nodiscard]] inline const RUNTIME_DATA2& GetRuntimeData2() const noexcept
		{
			return REL::RelocateMemberIfNewer<RUNTIME_DATA2>(SKSE::RUNTIME_SSE_1_6_1130, this, 0x140, 0x148);
		}

		[[nodiscard]] static inline TESWorldSpace* GetCurrentWorldSpace() noexcept
		{
			return GetSingleton()->GetRuntimeData2().worldSpace;
		}

		TESObjectCELL*  GetCell(const NiPoint3& a_position) const;
		MATERIAL_ID     GetLandMaterialType(const NiPoint3& a_position) const;
		bool            GetLandHeight(const NiPoint3& a_positionIn, float& a_heightOut);
		TESLandTexture* GetLandTexture(const NiPoint3& a_position) const;
		float           GetWaterHeight(const NiPoint3& a_pos, TESObjectCELL* a_cell) const;
		NiAVObject*     Pick(bhkPickData& a_pickData);
		void            PurgeBufferedCells();

		// members
		std::uint64_t                                       unk070;                     // 070
		GridCellArray*                                      gridCells;                  // 078
		NiNode*                                             objRoot;                    // 080
		NiNode*                                             lodLandRoot;                // 088
		NiNode*                                             objLODWaterRoot;            // 090
		BSTempNodeManager*                                  tempNodeManager;            // 098
		std::uint64_t                                       unk0A0;                     // 0A0
		std::uint64_t                                       unk0A8;                     // 0A8
		std::uint32_t                                       unk0B0;                     // 0B0
		std::uint32_t                                       unk0B4;                     // 0B4
		std::uint64_t                                       unk0B8;                     // 0B8
		TESObjectCELL*                                      interiorCell;               // 0C0
		TESObjectCELL**                                     interiorBuffer;             // 0C8
		TESObjectCELL**                                     exteriorBuffer;             // 0D0
		std::uint64_t                                       unk0D8;                     // 0D8
		std::uint64_t                                       unk0E0;                     // 0E0
		std::uint64_t                                       unk0E8;                     // 0E8
		std::uint64_t                                       unk0F0;                     // 0F0
		std::uint64_t                                       unk0F8;                     // 0F8
		Sky*                                                sky;                        // 100
		BSSimpleList<NiPointer<ImageSpaceModifierInstance>> activeImageSpaceModifiers;  // 108
		std::uint64_t                                       unk118;                     // 118
		std::uint64_t                                       unk120;                     // 120
	private:
		std::uint8_t _padRuntimeData[0x2B8 - 0x128];  // 128 - use GetRuntimeData(), GetAERuntimeData() and GetRuntimeData2()
	};
	static_assert(sizeof(TES) == 0x2B8);
}
