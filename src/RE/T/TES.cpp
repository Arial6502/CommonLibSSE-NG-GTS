#include "RE/T/TES.h"

#include "RE/B/bhkPickData.h"
#include "RE/G/GridCellArray.h"
#include "RE/N/NiAVObject.h"
#include "RE/T/TESObjectCELL.h"
#include "RE/T/TESObjectREFR.h"
#include "RE/T/TESWorldSpace.h"

namespace RE
{
	TES* TES::GetSingleton()
	{
		REL::Relocation<TES**> singleton{ Offset::TES::Singleton };
		return *singleton;
	}

	void TES::ForEachCell(std::function<void(TESObjectCELL*)> a_callback) const
	{
		if (interiorCell) {
			a_callback(interiorCell);
		} else {
			if (const auto gridLength = gridCells ? gridCells->length : 0; gridLength > 0) {
				std::uint32_t x = 0;
				do {
					std::uint32_t y = 0;
					do {
						if (const auto cell = gridCells->GetCell(x, y); cell && cell->IsAttached()) {
							a_callback(cell);
						}
						++y;
					} while (y < gridLength);
					++x;
				} while (x < gridLength);
			}
		}
		if (TESWorldSpace* const ws = GetCurrentWorldSpace()) {
			if (TESObjectCELL* const skyCell = ws ? ws->GetSkyCell() : nullptr; skyCell) {
				return a_callback(skyCell);
			}
		}
	}

	void TES::ForEachCellInRange(TESObjectREFR* a_origin, float a_radius, std::function<void(TESObjectCELL*)> a_callback) const
	{
		if (a_origin && a_radius > 0.0f) {
			const auto originPos = a_origin->GetPosition();

			if (interiorCell) {
				a_callback(interiorCell);
			} else {
				if (const auto gridLength = gridCells ? gridCells->length : 0; gridLength > 0) {
					const float yPlus = originPos.y + a_radius;
					const float yMinus = originPos.y - a_radius;
					const float xPlus = originPos.x + a_radius;
					const float xMinus = originPos.x - a_radius;

					std::uint32_t x = 0;
					do {
						std::uint32_t y = 0;
						do {
							if (const auto cell = gridCells->GetCell(x, y); cell && cell->IsAttached()) {
								if (const auto cellCoords = cell->GetCoordinates(); cellCoords) {
									const NiPoint2 worldPos{ cellCoords->worldX, cellCoords->worldY };
									if (worldPos.x < xPlus && (worldPos.x + 4096.0f) > xMinus && worldPos.y < yPlus && (worldPos.y + 4096.0f) > yMinus) {
										return a_callback(cell);
									}
								}
							}
							++y;
						} while (y < gridLength);
						++x;
					} while (x < gridLength);
				}
			}

			if (TESWorldSpace* const ws = GetCurrentWorldSpace()) {
				if (TESObjectCELL* const skyCell = ws ? ws->GetSkyCell() : nullptr; skyCell) {
					return a_callback(skyCell);
				}
			}
		} else {
			return ForEachCell(a_callback);
		}
	}

	void TES::ForEachReference(const std::function<BSContainer::ForEachResult(TESObjectREFR* a_ref)>& a_callback) const
	{
		if (interiorCell) {
			interiorCell->ForEachReference([&](TESObjectREFR* a_ref) {
				return a_callback(a_ref);
			});
		} else {
			if (const auto gridLength = gridCells ? gridCells->length : 0; gridLength > 0) {
				std::uint32_t x = 0;
				do {
					std::uint32_t y = 0;
					do {
						if (const auto cell = gridCells->GetCell(x, y); cell && cell->IsAttached()) {
							cell->ForEachReference([&](TESObjectREFR* a_ref) {
								return a_callback(a_ref);
							});
						}
						++y;
					} while (y < gridLength);
					++x;
				} while (x < gridLength);
			}
		}
		if (TESWorldSpace* const ws = GetCurrentWorldSpace()) {
			if (TESObjectCELL* const skyCell = ws ? ws->GetSkyCell() : nullptr; skyCell) {
				skyCell->ForEachReference([&](TESObjectREFR* a_ref) {
					return a_callback(a_ref);
				});
			}
		}
	}

	void TES::ForEachReferenceInRange(TESObjectREFR* a_origin, float a_radius, const std::function<BSContainer::ForEachResult(TESObjectREFR* a_ref)>& a_callback) const
	{
		if (a_origin && a_radius > 0.0f) {
			const auto  originPos = a_origin->GetPosition();

			if (interiorCell) {
				interiorCell->ForEachReferenceInRange(originPos, a_radius, [&](TESObjectREFR* a_ref) {
					return a_callback(a_ref);
				});
			} else {
				if (const auto gridLength = gridCells ? gridCells->length : 0; gridLength > 0) {
					const float yPlus = originPos.y + a_radius;
					const float yMinus = originPos.y - a_radius;
					const float xPlus = originPos.x + a_radius;
					const float xMinus = originPos.x - a_radius;

					std::uint32_t x = 0;
					do {
						std::uint32_t y = 0;
						do {
							if (const auto cell = gridCells->GetCell(x, y); cell && cell->IsAttached()) {
								if (const auto cellCoords = cell->GetCoordinates(); cellCoords) {
									const NiPoint2 worldPos{ cellCoords->worldX, cellCoords->worldY };
									if (worldPos.x < xPlus && (worldPos.x + 4096.0f) > xMinus && worldPos.y < yPlus && (worldPos.y + 4096.0f) > yMinus) {
										cell->ForEachReferenceInRange(originPos, a_radius, [&](TESObjectREFR* a_ref) {
											return a_callback(a_ref);
										});
									}
								}
							}
							++y;
						} while (y < gridLength);
						++x;
					} while (x < gridLength);
				}
			}

			if (TESWorldSpace* const ws = GetCurrentWorldSpace()) {
				if (TESObjectCELL* const skyCell = ws ? ws->GetSkyCell() : nullptr; skyCell) {
					skyCell->ForEachReferenceInRange(originPos, a_radius, [&](TESObjectREFR* a_ref) {
						return a_callback(a_ref);
					});
				}
			}
		} else {
			ForEachReference([&](TESObjectREFR* a_ref) {
				return a_callback(a_ref);
			});
		}
	}

	TESObjectCELL* TES::GetCell(const NiPoint3& a_position) const
	{
		using func_t = decltype(&TES::GetCell);
		REL::Relocation<func_t> func{ RELOCATION_ID(13177, 13322) };
		return func(this, a_position);
	}

	MATERIAL_ID TES::GetLandMaterialType(const NiPoint3& a_position) const
	{
		using func_t = decltype(&TES::GetLandMaterialType);
		REL::Relocation<func_t> func{ RELOCATION_ID(13203, 13349) };
		return func(this, a_position);
	}

	bool TES::GetLandHeight(const NiPoint3& a_positionIn, float& a_heightOut)
	{
		using func_t = decltype(&TES::GetLandHeight);
		REL::Relocation<func_t> func{ RELOCATION_ID(13198, 13344) };
		return func(this, a_positionIn, a_heightOut);
	}

	TESLandTexture* TES::GetLandTexture(const NiPoint3& a_position) const
	{
		using func_t = decltype(&TES::GetLandTexture);
		REL::Relocation<func_t> func{ RELOCATION_ID(13202, 13348) };
		return func(this, a_position);
	}

	float TES::GetWaterHeight(const NiPoint3& a_pos, TESObjectCELL* a_cell) const
	{
		using func_t = decltype(&TES::GetWaterHeight);
		REL::Relocation<func_t> func{ RELOCATION_ID(13212, AE_CHECK(SKSE::RUNTIME_SSE_1_7_99, 13358, 523463)) };
		return func(this, a_pos, a_cell);
	}

	NiAVObject* TES::Pick(bhkPickData& a_pickData)
	{
		using func_t = decltype(&TES::Pick);
		REL::Relocation<func_t> func{ RELOCATION_ID(13221, 13371) };
		return func(this, a_pickData);
	}

	void TES::PurgeBufferedCells()
	{
		using func_t = decltype(&TES::PurgeBufferedCells);
		REL::Relocation<func_t> func{ RELOCATION_ID(13159, 13299) };
		return func(this);
	}
}
