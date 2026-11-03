"""Layout facts from one Skyrim binary, for scripts/layout/diff_facts.py.

Runs inside Ghidra (PyGhidra) through the MCP eval_python tool, once per program:

    exec(open(r"<repo>/scripts/layout/ghidra_facts.py").read())
    run(r"<classes.json>", r"<out.json>", start, count)

classes.json comes from scripts/layout/classes.py. For every class it lists the AE and
SE vtable ids. The epoch of the open program picks which id is used. For each vtable this
writes the slot targets as address library ids, the RTTI base class offsets, and for each
slot function the ordered list of [this + disp] displacements it touches.
"""
import json
import re
import sys

sys.path.insert(0, r"C:\Users\Alpha\ghidra_scripts")
import bingus_helper as bh  # noqa: E402

_MEM = re.compile(r"\[(R[0-9A-Z]+)(?:\s*\+\s*0x([0-9a-fA-F]+))?\]")
_VOLATILE = {"RAX", "RCX", "RDX", "R8", "R9", "R10", "R11"}
_NONVOLATILE = {"RBX", "RBP", "RSI", "RDI", "R12", "R13", "R14", "R15"}
_PROGRAM = currentProgram  # noqa: F821  injected by Ghidra
_ROW = bh.row(_PROGRAM)
_MEMORY = _PROGRAM.getMemory()
_FUNCS = _PROGRAM.getFunctionManager()
_REFS = _PROGRAM.getReferenceManager()
_BASE = _PROGRAM.getImageBase().getOffset()
_DISP_CACHE = {}


def _addr(value):
    return bh.addr(_PROGRAM, value)


def _u32(a):
    return _MEMORY.getInt(a) & 0xFFFFFFFF


def _i32(a):
    return _MEMORY.getInt(a)


def _u64(a):
    return _MEMORY.getLong(a) & 0xFFFFFFFFFFFFFFFF


def _id_of(address):
    anchor = bh.al.anchor_at(_ROW, address)
    if anchor is None or anchor.offset:
        return None
    return anchor.id


def id_addr(id_):
    return bh.id_addr(_PROGRAM, id_)


def vtable_slots(vtable, limit=0x400):
    """Slot target addresses until the table ends. Every vtable with RTTI is preceded by
    its CompleteObjectLocator pointer, which points into .rdata, so the first entry that
    does not point at executable memory ends the table. Code references to single slots
    are common and say nothing about the end."""
    out = []
    for slot in range(limit):
        entry = vtable.add(slot * 8)
        target = _addr(_u64(entry))
        block = _MEMORY.getBlock(target)
        if block is None or not block.isExecute():
            break
        out.append(target)
    return out


def _instructions(target, limit=2000):
    """The function body at target, or when Ghidra has no function there, the listing
    from target up to the first RET."""
    func = _FUNCS.getFunctionAt(target)
    if func is not None:
        for ins in _PROGRAM.getListing().getInstructions(func.getBody(), True):
            yield ins
        return
    for n, ins in enumerate(_PROGRAM.getListing().getInstructions(target, True)):
        if n >= limit or (n and _FUNCS.getFunctionAt(ins.getAddress()) is not None):
            return
        yield ins
        if ins.getMnemonicString() == "RET":
            return


def rtti_bases(vtable):
    """[(mangled name, mdisp, pdisp, vdisp)] from the CompleteObjectLocator before the vtable."""
    col = _addr(_u64(vtable.add(-8)))
    if _u32(col) != 1:
        return None
    chd = _addr(_BASE + _u32(col.add(16)))
    count = _u32(chd.add(8))
    bca = _addr(_BASE + _u32(chd.add(12)))
    out = []
    for i in range(count):
        bcd = _addr(_BASE + _u32(bca.add(i * 4)))
        td = _addr(_BASE + _u32(bcd))
        name = []
        p = td.add(16)
        while len(name) < 512:
            c = _MEMORY.getByte(p) & 0xFF
            if not c:
                break
            name.append(chr(c))
            p = p.add(1)
        out.append(["".join(name), _i32(bcd.add(8)), _i32(bcd.add(12)), _i32(bcd.add(16))])
    return {"col_offset": _u32(col.add(4)), "bases": out}


def _copy_dest(ins, tracked):
    """The 64-bit register a `mov reg, reg` copies a tracked register into, else None."""
    if ins.getMnemonicString() != "MOV" or ins.getNumOperands() != 2:
        return None
    dst, src = ins.getOpObjects(0), ins.getOpObjects(1)
    if len(dst) != 1 or len(src) != 1 or "[" in ins.getDefaultOperandRepresentation(1):
        return None
    if not (hasattr(dst[0], "getBaseRegister") and hasattr(src[0], "getBaseRegister")):
        return None
    if src[0].getBaseRegister().getName() in tracked and dst[0].getBitLength() == 64:
        return dst[0].getBaseRegister().getName()
    return None


def _written(ins):
    return {o.getBaseRegister().getName() for o in ins.getResultObjects() if hasattr(o, "getBaseRegister")}


def this_disps(target):
    """Ordered [this + disp] displacements in the code at target.

    `this` arrives in RCX. Compilers keep it in one callee-saved register for the whole
    function, so a callee-saved register that copies RCX in the prologue (before the
    first branch or call) counts as `this` everywhere in the body. Volatile copies are
    followed linearly until clobbered. Any noise this lets in is the same code in every
    version, so it cannot fake a move."""
    key = target.getOffset()
    if key in _DISP_CACHE:
        return _DISP_CACHE[key]
    instructions = list(_instructions(target))

    tracked = {"RCX"}
    pinned = set()
    for ins in instructions:
        flow = ins.getFlowType()
        if flow.isCall() or flow.isJump() or flow.isConditional() or flow.isTerminal():
            break
        dest = _copy_dest(ins, tracked)
        tracked -= _written(ins)
        if dest:
            tracked.add(dest)
            if dest in _NONVOLATILE:
                pinned.add(dest)

    tracked = {"RCX"}
    out = []
    for ins in instructions:
        text = ins.toString()
        for m in _MEM.finditer(text):
            base = m.group(1)
            if (base in pinned or base in tracked) and "*" not in text[m.start():m.end()]:
                out.append(int(m.group(2) or "0", 16))
        dest = _copy_dest(ins, tracked | pinned)
        tracked -= _written(ins)
        if dest:
            tracked.add(dest)
        if ins.getFlowType().isCall():
            tracked -= _VOLATILE
    _DISP_CACHE[key] = out
    return out


def class_facts(entry, epoch):
    """Facts for one class: one record per vtable id the library lists."""
    tables = []
    for pair in entry["vtables"]:
        id_ = pair[1] if epoch == "ae" else pair[0]
        if not id_:
            tables.append(None)
            continue
        try:
            vt = id_addr(id_)
        except Exception:
            tables.append({"id": id_, "missing": True})
            continue
        slots = vtable_slots(vt)
        funcs = []
        for target in slots:
            funcs.append({"id": _id_of(target), "disps": this_disps(target)})
        tables.append({"id": id_, "rtti": rtti_bases(vt), "slots": funcs})
    # The library's own function ids for the class, keyed by the SE id (or the AE id when
    # there is no SE one) so that every runtime's record pairs up without a crosswalk.
    later = _vtuple(_ROW.version) >= (1, 7)
    funcs = []
    for se, ae16, ae17 in entry.get("funcs", []):
        id_ = (ae17 if later else ae16) if epoch == "ae" else se
        if not id_:
            continue
        try:
            target = id_addr(id_)
        except Exception:
            continue
        funcs.append({"id": se or -ae16, "disps": this_disps(target)})
    if funcs:
        tables.append({"id": "funcs", "kind": "funcs", "rtti": {"col_offset": 0, "bases": []}, "slots": funcs})
    return {"name": entry["name"], "tables": tables}


def _vtuple(version):
    return tuple(int(p) for p in str(version).split("."))


def run(classes_path, out_path, start=0, count=None, mode="a"):
    classes = json.load(open(classes_path))
    epoch = _ROW.epoch
    end = len(classes) if count is None else min(len(classes), start + count)
    with open(out_path, mode) as out:
        for entry in classes[start:end]:
            try:
                out.write(json.dumps(class_facts(entry, epoch)) + "\n")
            except Exception as e:
                out.write(json.dumps({"name": entry["name"], "error": str(e)}) + "\n")
    print("wrote %d classes [%d, %d) for %s %s" % (end - start, start, end, epoch, _ROW.version))
    return end
