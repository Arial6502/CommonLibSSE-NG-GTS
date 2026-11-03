"""Check the flatrim library layout against the binary facts of each supported runtime.

    python scripts/layout/check_library.py lib_flat.json facts_se.jsonl facts_ae16.jsonl facts_ae17.jsonl \
        moves_se_ae16.json moves_ae16_ae17.json out.tsv

lib_flat.json is parse_layout.py output for a flatrim (SE + AE) compile. A flatrim build
has one compiled layout, so anything it leaves visible must hold on every runtime:

  vtable  the slot count of each vtable subobject equals the binary's
  base    every base class sits at the offset RTTI gives for that runtime
  member  no visible member sits in a range the move detector (diff_facts.py --json)
          shows moving between two runtimes. Such a member has to sit behind an accessor.

Each row of out.tsv is one problem: class, check, runtime, detail.
"""
import json
import re
import sys

RUNTIMES = ("1.5.97", "1.6.1170", "1.7.104")


def leaf_of_mangled(mangled):
    """'.?AV?$BSTEventSink@VBSSystemEvent@@@@' -> 'BSTEventSink', '.?AVFoo@Bar@@' -> 'Foo'."""
    body = mangled[4:] if mangled[:4] in (".?AV", ".?AU") else mangled
    if body.startswith("?$"):
        body = body[2:]
    return body.split("@", 1)[0]


def leaf_of_lib(name):
    depth = 0
    out = []
    for c in name:
        if c == "<":
            depth += 1
        elif c == ">":
            depth -= 1
        elif depth == 0:
            out.append(c)
    return "".join(out).split("::")[-1].strip()


def load_facts(path):
    out = {}
    for line in open(path):
        rec = json.loads(line)
        out[rec["name"]] = rec
    return out


def lib_name(vtable_name):
    return "RE::" + vtable_name.replace("__", "::")


def main():
    lib = json.load(open(sys.argv[1]))
    facts = [load_facts(p) for p in sys.argv[2:5]]
    moves = [json.load(open(p)) for p in sys.argv[5:7]]
    out = open(sys.argv[7], "w", encoding="utf-8", newline="\n")
    out.write("class\tcheck\truntime\tdetail\n")
    problems = 0
    checked = 0

    for name in sorted(facts[0].keys() | facts[1].keys() | facts[2].keys()):
        cls = lib.get(lib_name(name))
        if cls is None:
            continue
        checked += 1
        for runtime, f in zip(RUNTIMES, facts):
            rec = f.get(name)
            if not rec or "error" in rec:
                continue
            binary_vt = {}
            for t in rec["tables"]:
                if t and not t.get("missing") and t.get("rtti") and t.get("kind") != "funcs":
                    binary_vt.setdefault(t["rtti"]["col_offset"], len(t["slots"]))
            for off, count in cls["vtables"].items():
                have = binary_vt.get(int(off))
                if have is not None and have != count:
                    out.write("%s\tvtable\t%s\tsubobject 0x%X: library %d slots, binary %d\n" % (name, runtime, int(off), count, have))
                    problems += 1
            primary = rec["tables"][0] if rec["tables"] and rec["tables"][0] and rec["tables"][0].get("kind") != "funcs" else None
            if primary and primary.get("rtti"):
                want = {}
                templated = set()
                for mangled, mdisp, _, _ in primary["rtti"]["bases"][1:]:
                    leaf = leaf_of_mangled(mangled)
                    want.setdefault(leaf, []).append(mdisp)
                    if mangled[4:6] == "?$":
                        templated.add(leaf)
                have = {}
                for off, _, base in cls["bases"]:
                    have.setdefault(leaf_of_lib(base), []).append(off)
                for leaf in sorted(set(want) & set(have)):
                    if sorted(set(want[leaf])) != sorted(set(have[leaf])):
                        # Container templates (BSTHashMap and kin) are modeled flatter than the
                        # game's empty trait bases. Their data offsets still agree, so they are
                        # reported apart from real base moves.
                        check = "base-template" if leaf in templated and not leaf.startswith(("BSTEvent", "BSTSingleton")) else "base"
                        out.write("%s\t%s\t%s\t%s: library %s, binary %s\n" % (
                            name, check, runtime, leaf, [hex(x) for x in sorted(set(have[leaf]))], [hex(x) for x in sorted(set(want[leaf]))]))
                        problems += 1
        for pair, report in zip(("1.5.97->1.6.1170", "1.6.1170->1.7.104"), moves):
            entry = next((e for e in report if e.get("name") == name), None)
            if not entry:
                continue
            for first, last, delta, votes in entry["moves"]:
                hit = [m for m in cls["members"] if first <= m[0] <= last]
                if hit:
                    sample = "; ".join("0x%X %s" % (m[0], m[2][-60:]) for m in hit[:4])
                    out.write("%s\tmember\t%s\tmoved 0x%X..0x%X by %+#x (%d votes), visible: %s%s\n" % (
                        name, pair, first, last, delta, votes, sample, " ..." if len(hit) > 4 else ""))
                    problems += 1
    print("checked %d classes, %d problems" % (checked, problems))


if __name__ == "__main__":
    main()
