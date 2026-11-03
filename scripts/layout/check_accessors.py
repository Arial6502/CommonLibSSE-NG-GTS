"""Check every member accessor in include/RE against the binary move runs.

    python scripts/layout/check_accessors.py <repo> moves_se_ae16.json moves_ae16_ae17.json out.tsv

moves_*.json are diff_facts.py --json outputs. An accessor promises how a block moves
between runtimes:

  RelocateMemberIfNewer<T>(SKSE::RUNTIME_SSE_x, this, a, b)
      moves by b - a across the gate x
  RelocateMember<T>(this, se, vr)
      does not move between SE and AE at all

A row is written when a move run overlaps the start of the block ([a, a + 0x40)) with a
different delta than promised, or when an accessor promises a move between two runtimes
and no run shows one there (unconfirmed, lower priority). The class is the header's file
name.
"""
import json
import os
import re
import sys

_IF_NEWER = re.compile(r"RelocateMemberIfNewer<[^;]*?>\(\s*SKSE::RUNTIME_SSE_(\d+)_(\d+)_(\d+)\s*,\s*(?:this|GetSingleton\(\))\s*,\s*(0x[0-9A-Fa-f]+|\d+)\s*,\s*(0x[0-9A-Fa-f]+|\d+)\s*\)")
_PLAIN = re.compile(r"RelocateMember<[^;]*?>\(\s*this\s*,\s*(0x[0-9A-Fa-f]+|\d+)\s*,\s*(0x[0-9A-Fa-f]+|\d+)\s*\)")
WINDOW = 0x40


def runs_by_class(path):
    return {e["name"]: e.get("moves", []) for e in json.load(open(path)) if "name" in e}


def overlapping(runs, a):
    return [r for r in runs if r[0] <= a + WINDOW - 1 and r[1] >= a]


def main():
    repo = sys.argv[1]
    se_ae = runs_by_class(sys.argv[2])
    ae_ae = runs_by_class(sys.argv[3])
    out = open(sys.argv[4], "w", encoding="utf-8", newline="\n")
    out.write("class\tfile\tline\taccessor\tproblem\n")
    count = 0
    for dirpath, _, files in os.walk(os.path.join(repo, "include", "RE")):
        for f in files:
            if not f.endswith(".h"):
                continue
            cls = f[:-2]
            path = os.path.join(dirpath, f)
            for lineno, line in enumerate(open(path, encoding="utf-8", errors="replace"), 1):
                for m in _IF_NEWER.finditer(line):
                    gate = tuple(int(m.group(i)) for i in (1, 2, 3))
                    a, b = int(m.group(4), 0), int(m.group(5), 0)
                    runs = (ae_ae if gate >= (1, 7, 0) else se_ae).get(cls, [])
                    other = (se_ae if gate >= (1, 7, 0) else ae_ae).get(cls, [])
                    hits = overlapping(runs, a)
                    if any(r[2] != b - a for r in hits):
                        problem = "binary moves %s" % ", ".join("0x%X..0x%X %+#x" % tuple(r[:3]) for r in hits)
                    elif not hits and runs and b != a:
                        problem = "unconfirmed: no run at 0x%X" % a
                    else:
                        problem = None
                    if not problem and overlapping(other, a):
                        problem = "also moves in the other version pair: %s" % ", ".join("0x%X..0x%X %+#x" % tuple(r[:3]) for r in overlapping(other, a))
                    if problem:
                        out.write("%s\t%s\t%d\t%s\t%s\n" % (cls, os.path.relpath(path, repo), lineno, m.group(0)[:90], problem))
                        count += 1
                for m in _PLAIN.finditer(line):
                    a = int(m.group(1), 0)
                    for label, table in (("SE->1.6.1170", se_ae), ("1.6.1170->1.7.104", ae_ae)):
                        hits = overlapping(table.get(cls, []), a)
                        if hits:
                            out.write("%s\t%s\t%d\t%s\tunversioned, but %s moves %s\n" % (
                                cls, os.path.relpath(path, repo), lineno, m.group(0)[:90], label,
                                ", ".join("0x%X..0x%X %+#x" % tuple(r[:3]) for r in hits)))
                            count += 1
    print("%d accessor problems" % count)


if __name__ == "__main__":
    main()
