"""Diff the layout facts of two binaries written by scripts/layout/ghidra_facts.py.

    python scripts/layout/diff_facts.py old.jsonl new.jsonl [--crosswalk crosswalk.tsv] [--only Name ...]

old and new are facts for two runtimes, for example AE 1.6.1170 and AE 1.7.104, or SE
1.5.97 and AE 1.6.1170. Within one epoch, slot functions pair by id. Across epochs they
pair through the crosswalk.

Per class this reports:
  - vtable slot changes (inserted or removed slots, by function id)
  - RTTI base offset changes
  - member moves: aligned [this + disp] accesses in paired functions, tallied as
    old offset -> new offset. A run of offsets that all move by the same delta is a
    member block that moved.
"""
import argparse
import collections
import difflib
import json


def load(path):
    out = {}
    for line in open(path):
        rec = json.loads(line)
        out[rec["name"]] = rec
    return out


def load_crosswalk(path):
    se2ae = {}
    for line in open(path):
        parts = line.rstrip("\n").split("\t")
        if len(parts) >= 2 and parts[0].isdigit() and parts[1].isdigit():
            se2ae[int(parts[0])] = int(parts[1])
    return se2ae


def slot_changes(old_slots, new_slots, pair):
    """difflib opcodes over the slot id sequences, ids mapped through pair."""
    a = [pair(s["id"]) for s in old_slots]
    b = [s["id"] for s in new_slots]
    ops = difflib.SequenceMatcher(None, a, b, autojunk=False).get_opcodes()
    # An equal-length replace is a pairing gap (unmapped id or thunk), not a slot change.
    return [op for op in ops if op[0] != "equal" and not (op[0] == "replace" and op[2] - op[1] == op[4] - op[3])]


def disp_moves(old_slots, new_slots, old_base, new_base, pair, tally):
    """Align the disp sequences of every slot pair with the same function and tally the
    old -> new offset of each replaced access. A secondary vtable's functions get `this`
    at their subobject, so old_base / new_base (the subobject offsets) turn their
    displacements into complete object offsets."""
    by_id = {}
    for s in new_slots:
        if s["id"] is not None:
            by_id.setdefault(s["id"], s)
    for s in old_slots:
        mate = by_id.get(pair(s["id"]))
        if mate is None:
            continue
        a = [d + old_base for d in s["disps"]]
        b = [d + new_base for d in mate["disps"]]
        for tag, i1, i2, j1, j2 in difflib.SequenceMatcher(None, a, b, autojunk=False).get_opcodes():
            if tag == "equal":
                for d in a[i1:i2]:
                    tally[(d, d)] += 1
            elif tag == "replace" and i2 - i1 == j2 - j1:
                for x, y in zip(a[i1:i2], b[j1:j2]):
                    tally[(x, y)] += 1


def summarize_moves(tally, min_votes=2):
    """old offset -> (new offset, votes, total votes for that old offset) for every old
    offset whose best supported new offset differs from it."""
    best = {}
    totals = collections.Counter()
    for (old, new), n in tally.items():
        totals[old] += n
        if old not in best or n > best[old][1]:
            best[old] = (new, n)
    out = []
    for old in sorted(best):
        new, n = best[old]
        if new != old and n >= min_votes:
            out.append((old, new, n, totals[old]))
    return out


def runs(moves):
    """Group consecutive moves with the same delta: [(first old, last old, delta, votes)]."""
    out = []
    for old, new, n, _ in moves:
        delta = new - old
        if out and out[-1][2] == delta:
            first, _, _, votes = out[-1]
            out[-1] = (first, old, delta, votes + n)
        else:
            out.append((old, old, delta, n))
    return out


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("old")
    ap.add_argument("new")
    ap.add_argument("--crosswalk")
    ap.add_argument("--only", nargs="*")
    ap.add_argument("--json")
    args = ap.parse_args()

    old, new = load(args.old), load(args.new)
    se2ae = load_crosswalk(args.crosswalk) if args.crosswalk else None
    pair = (lambda i: se2ae.get(i, -1 - (i or 0))) if se2ae else (lambda i: i)

    report = []
    for name in sorted(set(old) & set(new)):
        if args.only and name not in args.only:
            continue
        a, b = old[name], new[name]
        if "error" in a or "error" in b:
            report.append({"name": name, "error": a.get("error") or b.get("error")})
            continue
        entry = {"name": name, "slots": [], "bases": [], "moves": []}
        tally = collections.Counter()
        fa = next((t for t in a["tables"] if t and t.get("kind") == "funcs"), None)
        fb = next((t for t in b["tables"] if t and t.get("kind") == "funcs"), None)
        if fa and fb:
            disp_moves(fa["slots"], fb["slots"], 0, 0, lambda i: i, tally)
        vta = [t for t in a["tables"] if not (t and t.get("kind") == "funcs")]
        vtb = [t for t in b["tables"] if not (t and t.get("kind") == "funcs")]
        for i, (ta, tb) in enumerate(zip(vta, vtb)):
            if not ta or not tb or ta.get("missing") or tb.get("missing"):
                continue
            for op in slot_changes(ta["slots"], tb["slots"], pair):
                entry["slots"].append([i] + list(op))
            if i == 0 and ta["rtti"] and tb["rtti"]:
                ba = {n: m for n, m, _, _ in ta["rtti"]["bases"]}
                bb = {n: m for n, m, _, _ in tb["rtti"]["bases"]}
                for base in sorted(set(ba) | set(bb), key=lambda k: ba.get(k, bb.get(k))):
                    if ba.get(base) != bb.get(base):
                        entry["bases"].append([base, ba.get(base), bb.get(base)])
            if ta["rtti"] and tb["rtti"]:
                disp_moves(ta["slots"], tb["slots"], ta["rtti"]["col_offset"], tb["rtti"]["col_offset"], pair, tally)
        # A lone offset with a few votes is usually an alignment artifact, not a moved member.
        entry["moves"] = [r for r in runs(summarize_moves(tally)) if r[0] != r[1] or r[3] >= 4]
        if entry["slots"] or entry["bases"] or entry["moves"]:
            report.append(entry)

    if args.json:
        json.dump(report, open(args.json, "w"), indent=1)
    for e in report:
        if "error" in e:
            print("%s: error %s" % (e["name"], e["error"]))
            continue
        print("== %s" % e["name"])
        for base, x, y in e["bases"]:
            print("  base %-60s %s -> %s" % (base, hex(x) if x is not None else "-", hex(y) if y is not None else "-"))
        for vt, tag, i1, i2, j1, j2 in e["slots"]:
            print("  vtable %d slots %s old[%d:%d] new[%d:%d]" % (vt, tag, i1, i2, j1, j2))
        for first, last, delta, votes in e["moves"]:
            print("  moved 0x%X..0x%X by %+#x (%d votes)" % (first, last, delta, votes))
    print("%d classes differ" % len([e for e in report if "error" not in e]))


if __name__ == "__main__":
    main()
