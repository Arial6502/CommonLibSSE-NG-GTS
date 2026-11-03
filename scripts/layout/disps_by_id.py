"""Print the [this + disp] accesses of functions by address library id, in the open program.

Runs inside Ghidra through eval_python, after ghidra_facts.py has been exec'd:

    exec(open(r"<repo>/scripts/layout/ghidra_facts.py").read())
    exec(open(r"<repo>/scripts/layout/disps_by_id.py").read())
    show([(se_id, ae_id), ...])

The epoch of the open program picks se_id or ae_id. Run it in each of the three programs
and compare the lines: a member that moved shows up as a displacement that changed.
"""


def show(pairs):
    epoch = _ROW.epoch  # noqa: F821  from ghidra_facts.py
    for se_id, ae_id in pairs:
        id_ = ae_id if epoch == "ae" else se_id
        try:
            target = id_addr(id_)  # noqa: F821
        except Exception:
            print("%s %d: not in library" % (_ROW.version, id_))  # noqa: F821
            continue
        disps = this_disps(target)  # noqa: F821
        print("%s %6d %s: %s" % (_ROW.version, id_, bh.describe(_PROGRAM, target).split(" ", 1)[1][:40],  # noqa: F821
                                 " ".join("%X" % d for d in sorted(set(disps)))))
