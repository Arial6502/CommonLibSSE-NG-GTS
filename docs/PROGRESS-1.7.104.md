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
| D9 | Member moves are detected from `[this + disp]` accesses, not decompiled types. A callee-saved register that copies RCX in the prologue counts as `this` for the whole function. | Fast enough for every class in all three binaries. The code is the same in each version, so noise it lets in cannot fake a move; only real offset changes differ. |
| D10 | Per class, the harness covers vtable slot functions and every function the AE Ghidra databases name `Class::...`, plus the library's own ids for the class. AE ids are shared by 1.6.1170 and 1.7.104. SE pairs through the crosswalk. | Covers classes without a vtable (ControlMap) and regions vfuncs never touch (TES). |
| D7 | GTSPlugin is never edited. Its build is meant to break where D1 hides members. | User choice. GTSPlugin has its own RelocationIDEx helpers for per-version ids and offsets. |

## Verification levers

All are committed scripts, so every result can be rerun.

| Lever | What it proves | Status |
|-------|----------------|--------|
| `scripts/audit_ids.py` | Every id the sources use resolves in each library. | done |
| `scripts/test_addresslib.cpp` | `REL::IDDatabase` loads all three real libraries, rejects wrong version and format, and maps ids to the addresses Ghidra shows. 17 checks. | done, 17/17 pass |
| Hook site check (GTSPlugin `write_call` sites) | The instruction at each AE `id + offset` is still the same call in 1.7.104. | skipped (D7) |
| Library layout dump (`/d1reportAllClassLayout`, SE-only and AE-only configs) | What this library thinks every class looks like: size, bases, members. | running |
| Community PDB diff (D8) | Library AE layout vs the 1.6.1170 PDB: sizes and member offsets. Each mismatch is checked in the binary. | deprioritized. The PDB is derived from commonlib (SE names like `pad444` at AE 0x44C), so it is not independent. |
| `scripts/layout/ghidra_facts.py` + `classes.py` | Per binary, for every class in `Offsets_VTABLE.h`: vtable slots as ids, RTTI base offsets (exact), and the `[this + disp]` accesses of every slot function. | written. Full run over 7729 classes in progress. |
| `scripts/layout/diff_facts.py` | Diffs two binaries' facts. Reports base offset changes, slot insertions, and runs of member offsets that moved by the same delta. Secondary vtable accesses are normalized by the COL offset. | written. Reproduces GG's PlayerCharacter (+8 from 0x2D8) and SkyrimVM (+0x10 from 0x180) from the binaries alone, and the known 1.6.629 Actor +8. |
| Object size per binary | Allocation size feeding each constructor, where one can be found. | todo |
| Layout diff | Library dump vs the binary facts. Every mismatch becomes a finding below. | todo |

## Phases

- [x] P0. Survey this repo, GG, GTSPlugin, the Ghidra setup.
- [x] P0. Scope questions answered (D1, D6, verification depth).
- [x] P1. `scripts/audit_ids.py`.
- [x] P1. Hook site check for GTSPlugin. skip: D7.
- [x] P2. REL. Format 5 loader. Minor version >= 6 classified as AE. `RUNTIME_SSE_1_7_99` / `RUNTIME_SSE_1_7_104`. `Module::IsAtLeast`. GG's `report_and_error` return fixes. `AE_CHECK`.
- [x] P2. Fix the ids from F1, F2 and F7.
- [ ] P3. Layout harness. Library dump, RTTI bases, vtable diff, sizes, shift detector, diff report.
  - [x] Fact extractor and differ, validated on PlayerCharacter, SkyrimVM, Actor.
  - [x] Tracker fixed twice: vtable end detection (stop only at non-executable targets), and `this` held in callee-saved registers.
  - [x] Class list grows to 9645 classes (7999 with named functions) from Offsets_VTABLE.h, Offsets.h, class sources and Ghidra names.
  - [ ] Full fact runs: SE 1.5.97, AE 1.6.1170, AE 1.7.104.
  - [x] Library layout dumps (SE-only, AE-only, flatrim). 3965 RE classes each.
  - [ ] Attribute each detected move to the most-base class that shows it, so derived classes don't repeat it.
  - [ ] Review list: per class, binary moves vs what the library models. Every class lands in fixed, already-correct, or unverifiable.
  - [ ] Object sizes per binary (allocation sites).
- [ ] P4. 1.6.1170 parity. Port GG's 1.6.1130 changes and verify each one in Ghidra.
  - [x] ControlMap (F10)
  - [x] BGSSaveLoadManager (F11)
  - [x] TESContainer: only fills padding at 0x14, nothing moves. skip.
  - [x] Script id (F1)
  - [x] TES (F14)
  - [x] Renderer: GTS does not model the render target data (only the older BSRenderManager), nothing to fix. The harness covers BSRenderManager.
  - [x] BSSystemUtility, BSWin32SystemUtility: not in GTS. skip.
  - [x] BGSDefaultObjectManager count and 1.7 remap (F12, F13)
  - [x] Console / BSScaleformTranslator ids: SelectedRef already uses 504099 (1130+). GTS has no GetCommandScriptParentQuest or translator call.
- [ ] P4. Fix every 1.6.1170 mismatch the layout harness reports.
- [ ] P5. 1.7.104. Port and verify GG's changes. Done so far: SkyrimVM (F15).
  Done: PlayerCharacter (F16), MenuEventHandler / PlayerInputHandler / HeldStateHandler slots (F18).
  Remaining: SkyrimVM (+0x10 at 0x180), PlayerCharacter (+8 at 0x2D8), MenuEventHandler / PlayerInputHandler (+2 slots), HeldStateHandler, BSInputEventQueue, AttackBlockHandler, LookHandler, LockpickingMenu, BookMenu, bhkWorld, TESTopicInfo, BSSystemUtility (+1 slot), BSSaveDataSystemUtility (+6 slots), BGSDefaultObjectManager remap, new input event classes.
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
| F10 | ControlMap: from 1.6.1130 a new input context grows `controlMap[]` by one, so every field from `linkedMappings` on is +8. The library had them as plain members at SE offsets. | `disps_by_id.py` on 11 ControlMap functions (GG ids): e.g. ToggleControls touches 0x118/0x11C on SE and 0x120/0x124 on 1170 and 1.7.104. | fixed: `RUNTIME_DATA` behind `GetRuntimeData()`, pad in the flatrim layout |
| F11 | BGSSaveLoadManager: from 1.6.1130, 0x48 bytes are inserted at 0x2B0. The save thread (`unk2B0`) and request queue (`unk370`) move to 0x2F8 / 0x3B8. Members up to 0x2A8 do not move. | `Save_Impl` reads the thread at 0x2B0 on SE and 0x2F8 on 1170 and 1.7.104. GenerateCharacterID, PopulateSaveList and LoadMostRecentSaveGame touch the same offsets on all three. | fixed: `RUNTIME_DATA` behind `GetRuntimeData()` |
| F12 | BGSDefaultObjectManager: the initialized flags follow the form array, which holds 364 objects on SE, 366 on 1.6.1170 and 372 on 1.7.104, so the flags sit at 0xB80 / 0xB90 / 0xBC0. The library read 0xB80 on every runtime, so `IsObjectInitialized` and `GetObject` were wrong on 1.6.1170 already. 1.7.99 also inserts objects at 188 and 264-268, which renumbers later indices. | `InitializeAfterAllFormsRead` touches 0xB80 / 0xB90 / 0xBC0. 39 code references into `gBGSDefaultObjectManager.objects[]`, paired by function id 1170 vs 1.7.104: indices to 185 unchanged, 188-216 +1, 278-348 +6. The library enum puts the inserts after `kHelpManualXBox` (187) and `kHelpAttackTarget` (262), which fixes the +6 boundary at 263. | fixed: `ObjectCount()`, flags at `0x20 + count * 8`, 1.7 remap in `MapIndex`, `objectInit[]` hidden |
| F13 | `BGSDefaultObjectManager::GetObject(DefaultObjectID)` returned `&RelocateMember<TESForm**>(this, 0x20, 0x20)[idx]`, which reads `objects[0]` as a pointer and indexes off it. Wrong on every runtime. | Code reading | fixed: `&objects[idx]` |
| F14 | TES from 1.6.1130: the SE `showLANDborders` byte at 0x129 became a dword at 0x12C with a new dword after it, the bytes 0x12A-0x133 moved +0xA, the floats at 0x134/0x138 (declared u32/u64) moved +0xC, and everything from `worldSpace` (0x140) moved +8. GG's AE model keeps 0x134-0x13C in place and is wrong there. Members 0x70-0x120 do not move. | Disassembly of 4 paired functions (SE 13171/13148/13276/13140 vs AE 13316/13288/13427/13280): byte compares 0x12D/0x12E/0x132 -> 0x137/0x138/0x13C, MOVSS/COMISS 0x134/0x138 -> 0x140/0x144, MOVZX byte 0x129 -> MOV dword 0x12C, MOV byte 0x12A -> 0x134. 120 votes for +8 from 0x140. | fixed: `GetRuntimeData()` (SE, <1130), `GetAERuntimeData()` (1130+), `GetRuntimeData2()` (0x140 / 0x148) |
| F15 | SkyrimVM on 1.7.99+: `BSTEventSink<TESAmiiboTouchEvent>` and `BSTEventSink<TESAmiiboForcedStopDetectionEvent>` are inserted at 0x180, so the four later sinks, the singleton tag, the StatsEvent source and every member (from `impl` at 0x200) move +0x10. | RTTI base list of 1.6.1170 vs 1.7.104 (`diff_facts.py`). Member detector: +0x10 from 0x200 to 0x8970, 220 votes. Matches GG's 61 `impl` call sites at 0x200 -> 0x210. | fixed: members in `RUNTIME_DATA` behind `GetRuntimeData()`; the moved bases are no longer C++ bases but `As*()` accessors, so an upcast cannot compile to the wrong offset |
| F16 | PlayerCharacter on 1.7.99+: `BSTEventSink<BSSystemEvent>` is inserted at 0x2D8, so the three event sources and everything after them move +8. The four sinks before 0x2D8 do not move. | RTTI base list: sink at 0x2D8, BGSActorCell/Death/PositionPlayer sources 0x2D8/0x330/0x388 -> 0x2E0/0x338/0x390. Member detector 1.6.1170 -> 1.7.104: +8 from 0x2D8 through 0xBE5, 500+ votes. | fixed: one `Member<T>(se, ae, vr)` helper behind every accessor, `AsBSSystemEventSink()` |
| F17 | `PlayerCharacter::AsBGSActorDeathEventSource() const` used SE 0x2D0 (the cell event source) instead of 0x328. | Code reading, against the non-const overload and RTTI | fixed |
| F18 | MenuEventHandler and PlayerInputHandler on 1.7.99+: two slots (ProcessMotionGesture, ProcessSixaxis) are inserted after CanProcess, so every later slot moves +2, including HeldStateHandler's pair (05/06 -> 07/08). Slot 01 does not move. | Vtable ids 1.6.1170 vs 1.7.104. LookHandler: MouseMove override 42424 at 3 -> 5, default Button 33130 at 4 -> 6. MenuOpenHandler: Kinect 52250 at 2 -> 4, Button 52249 at 5 -> 7. ActivateHandler: UpdateHeldStateActive / SetHeldStateActive 42335 / 42452 at 5 / 6 -> 7 / 8. | fixed: with AE enabled the shifted functions are non-virtual and call the runtime slot. Their overrides in 30 derived headers are compiled out with AE, since no compile time vtable fits both layouts. A plugin's own subclass that overrides them stops compiling instead of crashing on 1.7. |
