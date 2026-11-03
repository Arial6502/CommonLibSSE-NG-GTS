"""Audit the address library ids a source tree references against the SE, AE 1.6.1170 and
AE 1.7.104 address libraries.

    python scripts/audit_ids.py <addresslib dir> <source dir>... [--tsv out.tsv]

The addresslib dir holds version-1-5-97-0.bin, versionlib-1-6-1170-0.bin and
versionlib-1-7-104-0.bin. Every id pair found in the sources is reported with whether
it resolves in each library; ids that resolve in 1.6.1170 but not in 1.7.104 are the
ones that fail at runtime on 1.7.104.
"""
import argparse
import os
import re
import struct
import sys

LIBS = {
    "1.5.97": "version-1-5-97-0.bin",
    "1.6.1170": "versionlib-1-6-1170-0.bin",
    "1.7.104": "versionlib-1-7-104-0.bin",
}


def _delta(data, pos, ptr_size, count):
    out = {}
    prev_id = prev_off = 0
    for _ in range(count):
        t = data[pos]
        pos += 1
        lo, hi = t & 0xF, t >> 4

        def take(fmt):
            nonlocal pos
            v = struct.unpack_from(fmt, data, pos)[0]
            pos += struct.calcsize(fmt)
            return v

        id_ = {
            0: lambda: take("<Q"),
            1: lambda: prev_id + 1,
            2: lambda: prev_id + take("<B"),
            3: lambda: prev_id - take("<B"),
            4: lambda: prev_id + take("<H"),
            5: lambda: prev_id - take("<H"),
            6: lambda: take("<H"),
            7: lambda: take("<I"),
        }[lo]()
        tmp = prev_off // ptr_size if hi & 8 else prev_off
        off = {
            0: lambda: take("<Q"),
            1: lambda: tmp + 1,
            2: lambda: tmp + take("<B"),
            3: lambda: tmp - take("<B"),
            4: lambda: tmp + take("<H"),
            5: lambda: tmp - take("<H"),
            6: lambda: take("<H"),
            7: lambda: take("<I"),
        }[hi & 7]()
        if hi & 8:
            off *= ptr_size
        out[id_] = off
        prev_id, prev_off = id_, off
    return out


def load_lib(path):
    """id -> rva for meh321 formats 1, 2 (delta coded) and 5 (dense u32 array)."""
    data = open(path, "rb").read()
    fmt = struct.unpack_from("<i", data, 0)[0]
    if fmt == 5:
        ptr_size, _, count = struct.unpack_from("<III", data, 0x54)
        assert ptr_size == 8 and len(data) == 0x60 + count * 4, path
        rvas = struct.unpack_from("<%dI" % count, data, 0x60)
        return {i: r for i, r in enumerate(rvas) if r}
    if fmt in (1, 2):
        name_len = struct.unpack_from("<i", data, 0x14)[0]
        pos = 0x18 + name_len
        ptr_size, count = struct.unpack_from("<ii", data, pos)
        return _delta(data, pos + 8, ptr_size, count)
    raise ValueError("unknown address library format %d in %s" % (fmt, path))


_NUM = r"\s*(0x[0-9A-Fa-f]+|\d+|NULL|nullptr)\s*"
# (kind, regex). Every pattern captures (se, ae); kind tells the reader which macro it was.
PATTERNS = [
    ("RelocationID", re.compile(r"(?:REL::)?RelocationID\(" + _NUM + "," + _NUM + r"[,)]")),
    ("RELOCATION_ID", re.compile(r"RELOCATION_ID\(" + _NUM + "," + _NUM + r"\)")),
    ("VariantID", re.compile(r"(?:REL::)?VariantID\(" + _NUM + "," + _NUM + ",")),
]
BARE_ID = re.compile(r"REL::ID\(" + _NUM + r"\)")
SOURCE_EXT = (".h", ".hpp", ".hxx", ".inl", ".cpp", ".cxx", ".cc")


def _int(text):
    text = text.strip()
    if text in ("NULL", "nullptr"):
        return 0
    return int(text, 0)


def scan(roots):
    """Yield (path, line, kind, se, ae) for every id reference under roots."""
    for root in roots:
        for dirpath, dirnames, filenames in os.walk(root):
            dirnames[:] = [d for d in dirnames if d not in ("build", "out", "extern", ".git", "vcpkg_installed")]
            for name in filenames:
                if not name.endswith(SOURCE_EXT):
                    continue
                path = os.path.join(dirpath, name)
                with open(path, encoding="utf-8", errors="replace") as f:
                    for lineno, line in enumerate(f, 1):
                        stripped = line.lstrip()
                        if stripped.startswith("//"):
                            continue
                        for kind, rx in PATTERNS:
                            for m in rx.finditer(line):
                                yield path, lineno, kind, _int(m.group(1)), _int(m.group(2))
                        for m in BARE_ID.finditer(line):
                            yield path, lineno, "ID", None, _int(m.group(1))


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("libdir")
    ap.add_argument("roots", nargs="+")
    ap.add_argument("--tsv")
    args = ap.parse_args()

    libs = {v: load_lib(os.path.join(args.libdir, f)) for v, f in LIBS.items()}
    se, ae16, ae17 = libs["1.5.97"], libs["1.6.1170"], libs["1.7.104"]

    rows = []
    for path, lineno, kind, se_id, ae_id in scan(args.roots):
        if kind == "ID":
            # Epoch unknown: a bare REL::ID is the same number on every runtime.
            status = "bare-id"
        elif not ae_id:
            status = "no-ae-id"
        elif ae_id not in ae16 and ae_id not in ae17:
            status = "missing-ae"
        elif ae_id not in ae17:
            status = "missing-1.7.104"
        elif ae_id not in ae16:
            status = "missing-1.6.1170"
        else:
            status = "ok"
        se_status = "" if not se_id else ("ok" if se_id in se else "missing-se")
        rows.append((status, se_status, kind, se_id or 0, ae_id or 0, os.path.relpath(path), lineno))

    rows.sort(key=lambda r: (r[0] == "ok", r[0], r[5], r[6]))
    summary = {}
    for r in rows:
        summary[r[0]] = summary.get(r[0], 0) + 1
    se_missing = sum(1 for r in rows if r[1] == "missing-se")

    out = open(args.tsv, "w", encoding="utf-8", newline="\n") if args.tsv else None
    if out:
        out.write("status\tse_status\tkind\tse\tae\tfile\tline\n")
    for r in rows:
        line = "\t".join(str(x) for x in r)
        if out:
            out.write(line + "\n")
        elif r[0] not in ("ok", "no-ae-id") or r[1] == "missing-se":
            print(line)
    print("summary:", ", ".join("%s=%d" % kv for kv in sorted(summary.items())), "missing-se=%d" % se_missing, file=sys.stderr)


if __name__ == "__main__":
    main()
