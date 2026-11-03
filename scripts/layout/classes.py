"""Class list for scripts/layout/ghidra_facts.py.

    python scripts/layout/classes.py <repo> out.json [--ns ns_ae16.json ns_ae17.json --crosswalk crosswalk.tsv] [Name ...]

Each entry is {"name", "vtables": [[se, ae], ...], "funcs": [[se, ae16, ae17], ...]}.
vtables come from include/RE/Offsets_VTABLE.h in the library's order, so index 0 is the
primary vtable. funcs are the RELOCATION_ID pairs in the class's own header and source
(include/RE/X/<Name>.h, src/RE/X/<Name>.cpp), in its `Offset::<Name>` namespace in
Offsets.h, and, with --ns, every function the AE Ghidra databases name inside the
class's namespace (ghidra_namespaces.py), paired to SE through the crosswalk. Most are
member functions, so their `this` is the class. ae17 differs from ae16 only where AE_CHECK renumbers the id for
1.7. Classes without a vtable still get an entry when they have funcs. Names given on
the command line restrict the output.
"""
import json
import os
import re
import sys

_DECL = re.compile(r"VTABLE_(\w+)\s*\{(.*)\};")
_VID = re.compile(r"VariantID\(\s*(\d+)\s*,\s*(\d+)\s*,")
_N = r"\s*(\d+)\s*"
_RID = re.compile(r"(?:RELOCATION_ID|RelocationID)\(" + _N + "," + _N + r"[,)]")
_RID_CHECK = re.compile(r"RELOCATION_ID\(" + _N + r",\s*AE_CHECK\(\s*SKSE::RUNTIME_SSE_(\d+)_(\d+)_(\d+)\s*," + _N + "," + _N + r"\)")


def vtables(repo):
    out = {}
    for line in open(os.path.join(repo, "include", "RE", "Offsets_VTABLE.h"), encoding="utf-8"):
        m = _DECL.search(line)
        if m:
            out[m.group(1)] = [[int(a), int(b)] for a, b in _VID.findall(m.group(2))]
    return out


def funcs(repo):
    out = {}
    for sub, ext in (("include", ".h"), ("src", ".cpp")):
        root = os.path.join(repo, sub, "RE")
        for dirpath, _, files in os.walk(root):
            for f in files:
                if not f.endswith(ext) or f.startswith("Offsets"):
                    continue
                name = f[: -len(ext)]
                ids = out.setdefault(name, [])
                for line in open(os.path.join(dirpath, f), encoding="utf-8", errors="replace"):
                    if line.lstrip().startswith("//"):
                        continue
                    for m in _RID_CHECK.finditer(line):
                        se, gate, older, newer = int(m.group(1)), tuple(int(m.group(i)) for i in (2, 3, 4)), int(m.group(5)), int(m.group(6))
                        ae16 = older if (1, 6, 1170) < gate else newer
                        ae17 = older if (1, 7, 104) < gate else newer
                        ids.append([se, ae16, ae17])
                    for m in _RID.finditer(line):
                        ids.append([int(m.group(1)), int(m.group(2)), int(m.group(2))])
    return {k: v for k, v in out.items() if v}


def offsets_namespaces(repo, out):
    """RELOCATION_ID pairs in include/RE/Offsets.h, by their namespace under Offset."""
    stack = []
    for line in open(os.path.join(repo, "include", "RE", "Offsets.h"), encoding="utf-8"):
        stripped = line.strip()
        m = re.match(r"namespace\s+([\w:]+)", stripped)
        if m:
            stack.extend(m.group(1).split("::"))
            continue
        if stripped.startswith("}") and stack:
            # `namespace RE::Offset` opens two names with one brace.
            stack = stack[:-2] if stack[-2:] == ["RE", "Offset"] else stack[:-1]
            continue
        if "Offset" in stack:
            name = "::".join(stack[stack.index("Offset") + 1:])
            for m in _RID.finditer(line):
                if name:
                    out.setdefault(name.split("::")[-1], []).append([int(m.group(1)), int(m.group(2)), int(m.group(2))])


def ghidra_namespaces(paths, crosswalk_path, out):
    """Merge ghidra_namespaces.py dumps: AE ids per class, SE ids from the crosswalk."""
    ae2se = {}
    for line in open(crosswalk_path, encoding="utf-8"):
        parts = line.rstrip("\n").split("\t")
        if len(parts) >= 2 and parts[0].isdigit() and parts[1].isdigit():
            ae2se.setdefault(int(parts[1]), int(parts[0]))
    dumps = [json.load(open(p)) for p in paths]
    for ns in set().union(*dumps):
        ids = set().union(*(set(d.get(ns, [])) for d in dumps))
        leaf = ns.split("::")[-1] if "::" in ns and ns.split("::")[0] in ("BSResource", "BSScript", "SkyrimScript") else ns
        lst = out.setdefault(leaf.replace("::", "__"), [])
        for ae in sorted(ids):
            lst.append([ae2se.get(ae, 0), ae, ae])


def build(repo, ns_paths=None, crosswalk=None):
    vt = vtables(repo)
    fn = funcs(repo)
    offsets_namespaces(repo, fn)
    if ns_paths:
        ghidra_namespaces(ns_paths, crosswalk, fn)
    for name, ids in fn.items():
        seen = set()
        fn[name] = [i for i in ids if not (tuple(i) in seen or seen.add(tuple(i)))]
    names = list(vt) + sorted(set(fn) - set(vt))
    return [{"name": n, "vtables": vt.get(n, []), "funcs": fn.get(n, [])} for n in names]


if __name__ == "__main__":
    args = sys.argv[3:]
    ns_paths, crosswalk = None, None
    if args[:1] == ["--ns"]:
        ns_paths, args = args[1:3], args[3:]
    if args[:1] == ["--crosswalk"]:
        crosswalk, args = args[1], args[2:]
    classes = build(sys.argv[1], ns_paths, crosswalk)
    wanted = set(args)
    if wanted:
        classes = [c for c in classes if c["name"] in wanted]
    json.dump(classes, open(sys.argv[2], "w"))
    print(len(classes), "classes,", sum(1 for c in classes if c["funcs"]), "with funcs")
