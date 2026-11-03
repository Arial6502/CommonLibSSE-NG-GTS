"""Address library ids of every function that the open Ghidra program names inside a
class namespace, for scripts/layout/classes.py.

Runs inside Ghidra through eval_python:

    exec(open(r"<repo>/scripts/layout/ghidra_namespaces.py").read())
    dump(r"<out.json>")

Writes {"<class>": [id, ...]}. Names are mostly flat global symbols like "TES::sub",
sometimes real namespaces, so the class is everything before the last "::", without a
leading RE::.
"""
import json
import sys

sys.path.insert(0, r"C:\Users\Alpha\ghidra_scripts")
import bingus_helper as bh  # noqa: E402


def dump(out_path):
    program = currentProgram  # noqa: F821  injected by Ghidra
    row = bh.row(program)
    out = {}
    for func in program.getFunctionManager().getFunctions(True):
        # Most names are flat global symbols like "TES::sub", some are real namespaces.
        full = func.getName(True)
        if "::" not in full:
            continue
        parts = full.split("::")[:-1]
        if parts and parts[0] in ("RE", "std"):
            parts = parts[1:]
        if not parts:
            continue
        anchor = bh.al.anchor_at(row, func.getEntryPoint())
        if anchor is None or anchor.offset:
            continue
        out.setdefault("::".join(parts), []).append(anchor.id)
    json.dump(out, open(out_path, "w"))
    print("%d namespaces, %d functions" % (len(out), sum(len(v) for v in out.values())))
