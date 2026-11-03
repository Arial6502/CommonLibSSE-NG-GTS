"""Which default object index each function reads, from code references into the static
BGSDefaultObjectManager's objects[] array (gBGSDefaultObjectManager, AE data id 400191).

Runs inside Ghidra through eval_python, after ghidra_facts.py has been exec'd:

    exec(open(r"<repo>/scripts/layout/ghidra_default_objects.py").read())
    dump(r"<out.json>", count)

Writes {"<function id>": [index, ...]}. Run it on two AE builds and pair the lists by
function id to see how indices moved between them.
"""
import json


def dump(out_path, count, singleton_id=400191):
    base = id_addr(singleton_id).add(0x20)  # noqa: F821  from ghidra_facts.py
    out = {}
    for idx in range(count):
        for ref in _REFS.getReferencesTo(base.add(idx * 8)):  # noqa: F821
            func = _FUNCS.getFunctionContaining(ref.getFromAddress())  # noqa: F821
            if func is None:
                continue
            fid = _id_of(func.getEntryPoint())  # noqa: F821
            if fid is not None:
                out.setdefault(str(fid), []).append(idx)
    for v in out.values():
        v.sort()
    json.dump(out, open(out_path, "w"))
    print("%d functions, %d references" % (len(out), sum(len(v) for v in out.values())))
