"""Parse MSVC /d1reportAllClassLayout output into JSON.

    python scripts/layout/parse_layout.py layout.txt out.json

Only classes in namespace RE are kept. Per class:
  size     sizeof
  bases    [[offset, depth, name]] every base subobject, at its offset in this class
  members  [[offset, depth, text]] every data member line, depth 1 is a direct member
  vtables  {"<subobject offset>": slot count}
"""
import json
import re
import sys

_CLASS = re.compile(r"^class (RE::\S.*?)\tsize\((\d+)\):$")
_LINE = re.compile(r"^\s*(\d+)?\t((?:\| )*)(.*)$")
_BASE = re.compile(r"^\+--- \((?:virtual )?base class (.*)\)$")
# A class with one vfptr prints '$vftable@:', one with several '$vftable@Base@:' per subobject.
_VFTABLE = re.compile(r"^(RE::\S.*?)::\$vftable@(?:(.*)@)?:$")
_SLOT = re.compile(r"^\s*(\d+)\t\| &")
_ADJUST = re.compile(r"^\t\| (-?\d+)$")


def parse(path):
    classes = {}
    current = None
    vtable = None
    vtable_offset = None
    for raw in open(path, encoding="utf-8", errors="replace"):
        line = raw.rstrip("\n")
        m = _CLASS.match(line)
        if m:
            current = {"size": int(m.group(2)), "bases": [], "members": [], "vtables": {}}
            classes[m.group(1)] = current
            vtable = None
            continue
        m = _VFTABLE.match(line)
        if m:
            vtable = classes.get(m.group(1))
            vtable_offset = 0
            continue
        if vtable is not None:
            m = _ADJUST.match(line)
            if m:
                vtable_offset = -int(m.group(1))
                vtable["vtables"].setdefault(str(vtable_offset), 0)
                continue
            if _SLOT.match(line):
                key = str(vtable_offset)
                vtable["vtables"][key] = vtable["vtables"].get(key, 0) + 1
                continue
            if not line.strip():
                vtable = None
            continue
        if current is None:
            continue
        m = _LINE.match(line)
        if not m or m.group(1) is None:
            continue
        offset, depth, text = int(m.group(1)), m.group(2).count("|"), m.group(3).strip()
        b = _BASE.match(text)
        if b:
            current["bases"].append([offset, depth, b.group(1)])
        elif text and text != "+---" and not text.startswith("<alignment"):
            current["members"].append([offset, depth, text])
    return classes


if __name__ == "__main__":
    out = parse(sys.argv[1])
    json.dump(out, open(sys.argv[2], "w"))
    print(len(out), "RE classes")
