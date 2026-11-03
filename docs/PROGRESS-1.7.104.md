# 1.7.104 support and 1.6.1170 parity: progress

Branch `support/1.7.104`, cut from `master-3.6` at `b63e0d82`.

## Goal

One flatrim (SE + AE) build that is correct on exactly three runtimes: SE 1.5.97,
AE 1.6.1170 and AE 1.7.104. Other runtimes are not supported targets. VR stays off and
untested.

## Done means

1. The address library loader reads `versionlib-1-7-104-0.bin` (format 5) and still reads
   the SE format 1 and AE format 2 files.
2. `scripts/audit_ids.py` reports zero ids that are used on a supported runtime but
   missing from that runtime's library, for both this library and GTSPlugin.
3. Every class layout check in the layout harness (below) passes on all three runtimes,
   or the class is listed under "Known unverifiable" with the reason.
4. The flatrim build of this library compiles. GTSPlugin is not edited (D7). Its expected breaks get listed.
5. Nothing is claimed as verified in game. In-game testing is the user's call.

## Decisions

| # | Decision | Why |
|---|----------|-----|
| D1 | Shifted members become private pads, reached only through version aware accessors. | User choice. Direct reads of a shifted member then fail to compile instead of reading wrong memory on 1.7.104. |
| D2 | Port from CommonLibSSE-GG (MIT, branched before the upstream GPL relicense). Do not copy from GPL upstream. | License stays MIT. |
| D3 | 1.7 gates use `RUNTIME_SSE_1_7_99`. | GG verified 1.7.104 is a recompile of 1.7.99 with no layout change. The gate is correct for both. |
| D4 | Gates for the 1.6.1130 changes stay at `RUNTIME_SSE_1_6_1130`, not 1170. | That is where the change happened. It is correct for 1170 as well. |
| D5 | Ids that change in 1.7 use GG's `AE_CHECK(version, older, newer)`. | 1.7.104 gave some functions new ids (see F2). It is the same idiom GG already uses for 1.6.1130 id changes. |
| D6 | One local commit per verified unit. No push. | User choice. |
| D8 | `SkyrimSE.pdb` from the Crash Logger mod (`E:\Mods\ModOrganizer Skyrim AE\Data\Mods\Crash Logger\SKSE\Plugins`) is a 1.6.1170 PDB exported from Ghidra out of community RE work. It is a second opinion on member offsets, not ground truth. When it disagrees with this library, the binary decides. | About 32k struct records with member offsets. Nothing else gives per-member data at that scale. |
| D7 | GTSPlugin is never edited. Its build is meant to break where D1 hides members. | User choice. GTSPlugin has its own RelocationIDEx helpers for per-version ids and offsets. |

## Verification levers

All are committed scripts, so every result can be rerun.

| Lever | What it proves | Status |
|-------|----------------|--------|
| `scripts/audit_ids.py` | Every id the sources use resolves in each library. | done |
| `scripts/test_addresslib.cpp` | `REL::IDDatabase` loads all three real libraries, rejects wrong version and format, and maps ids to the addresses Ghidra shows. 17 checks. | done, 17/17 pass |
| Hook site check (GTSPlugin `write_call` sites) | The instruction at each AE `id + offset` is still the same call in 1.7.104. | skipped (D7) |
| Library layout dump (`/d1reportAllClassLayout`, SE-only and AE-only configs) | What this library thinks every class looks like: size, bases, members. | todo |
| Community PDB diff (D8) | Library AE layout vs the 1.6.1170 PDB: sizes and member offsets. Each mismatch is checked in the binary. | todo |
| RTTI base offsets per binary | Exact base class offsets for every polymorphic class, per runtime. | todo |
| Vtable slot diff per binary | Exact slot order. 1.6.1170 vs 1.7.104 compares ids directly. SE vs AE goes through the crosswalk. | todo |
| Object size per binary | Allocation size feeding each constructor, where one can be found. | todo |
| Member shift detector | Per class, `this + disp` accesses in matched functions across runtimes. A constant delta from some offset onward means members moved. | todo |
| Layout diff | Library dump vs the binary facts. Every mismatch becomes a finding below. | todo |

## Phases

- [x] P0. Survey this repo, GG, GTSPlugin, the Ghidra setup.
- [x] P0. Scope questions answered (D1, D6, verification depth).
- [x] P1. `scripts/audit_ids.py`.
- [x] P1. Hook site check for GTSPlugin. skip: D7.
- [x] P2. REL. Format 5 loader. Minor version >= 6 classified as AE. `RUNTIME_SSE_1_7_99` / `RUNTIME_SSE_1_7_104`. `Module::IsAtLeast`. GG's `report_and_error` return fixes. `AE_CHECK`.
- [x] P2. Fix the ids from F1, F2 and F7.
- [ ] P3. Layout harness. Library dump, RTTI bases, vtable diff, sizes, shift detector, diff report.
- [ ] P4. 1.6.1170 parity. Port GG's 1.6.1130 changes and verify each one in Ghidra: ControlMap, Renderer, BGSSaveLoadManager, TES, TESContainer, BSSystemUtility, BSWin32SystemUtility, BGSDefaultObjectManager count (366 on AE, not 364), Console / BSScaleformTranslator / Script ids.
- [ ] P4. Fix every 1.6.1170 mismatch the layout harness reports.
- [ ] P5. 1.7.104. Port and verify GG's changes: SkyrimVM (+0x10 at 0x180), PlayerCharacter (+8 at 0x2D8), MenuEventHandler / PlayerInputHandler (+2 slots), HeldStateHandler, BSInputEventQueue, AttackBlockHandler, LookHandler, LockpickingMenu, BookMenu, bhkWorld, TESTopicInfo, BSSystemUtility (+1 slot), BSSaveDataSystemUtility (+6 slots), BGSDefaultObjectManager remap, new input event classes.
- [ ] P5. Fix every 1.7.104 mismatch the layout harness reports beyond GG's list.
- [ ] P6. Build flatrim. Try a GTSPlugin build against it and list the breaks D1 causes, without editing GTSPlugin (D7).
- [ ] P7. Final rerun of every lever. Update this file.

## Findings

| # | Finding | Evidence | Status |
|---|---------|----------|--------|
| F1 | Already broken on 1.6.1170: `BSString::Set_CStr` (AE 11044) and `Script::CompileAndRun` (AE 21890) are not in the 1170 library. | `audit_ids.py`. 441582 is `Script::CompileAndRun_Impl` in the 1170 Ghidra db and exists in 1.7.104. | CompileAndRun fixed with `AE_CHECK(1_6_1130, 21890, 441582)`. Set_CStr has no users, won't fix. |
| F2 | 1.7.104 renumbered `TES::GetWaterHeight` from AE 13358 to 523463. | Both `Actor::SetParentCell` and `BGSProcedureWanderExecState` call it at the same offsets (+0x15E, +0xF0) in 1170 and 1.7.104. | fixed, `AE_CHECK(1_7_99, 13358, 523463)` |
| F3 | Two combat factory vtables (AE 211837, 211893) are gone in 1.7.104. Nothing in this library or GTSPlugin uses them. | `audit_ids.py` | won't fix |
| F4 | 685 AE vtable ids (mostly `bnet::` callbacks) exist in neither AE library. GG has the same entries. They are unused. | `audit_ids.py` | won't fix |
| F5 | GTSPlugin: all 70 ids resolve on all three runtimes. Hook offsets still unchecked. | `audit_ids.py` | open |
| F6 | GTSPlugin hardcodes AE call offsets (`REL::VariantOffset(se, ae, vr)`) in 24 `write_call` sites, plus vtable indices for Actor hooks. The compiler change can move both. | Extracted list in scratch. | user's (D7) |
| F7 | Bare `REL::ID(n)` resolves the same number on SE and AE. Four were AE ids that also ran on SE: `BGSAddToPlayerInventoryEvent::GetIndex`, `BGSStoryEventManager::GetSingleton` and `AddEvent_Impl`, `PlayerCharacter::AddPlayerAddItemEvent`. | SE twins from the crosswalk, each one checked in the SE Ghidra db and matching GG. | fixed |
| F8 | The other bare ids are fine in flatrim. `BSShaderTextureSet::Ctor` (SE 99886) is only called on the SE path of `Create()`, and AE inlines that ctor (memset, `NiObject::ctor`, vector ctor in the caller). `LooseFileStream` is compiled out when AE is on. Its AE ctor would be 70004 (same slot after `Allocate` and two `BSFixedString` inits in the matched caller). | Ghidra, SE 99849 vs AE 106494, SE 68636 vs AE 69981 | OK, not changed |
| F9 | `Logger` uses AE 380738 only below 1.6.1130. The log folder name id changed in 1130, so the gate is intentional. | User | OK |
