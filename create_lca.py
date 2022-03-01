from wrappedfst import WrappedFst
import os


def create_multiple_lca_txt(inf, outf, dct_isyms, write_syms_f, index_offset, cost):
    paths = []
    syms = {}
    syms['<eps>'] = 0
    with open(inf) as fh:
        for line in fh:
            path = line.split()
            for w in path:
                if w not in syms:
                    syms[w] = len(syms) + index_offset
            if dct_isyms:
                path = [dct_isyms[w] for w in path]
            paths.append(path)
    cost /= len(paths)
    output_lines = []
    start_state = 0
    end_state = 1
    state = 2
    for path in paths:
        assert len(path)
        if len(path) >= 3:
            s = f'{start_state} {state} {path[0]} {path[0]} {cost}'
            state += 1
            output_lines.append(s)
            for sym in path[1:-1]:
                s = f'{state - 1} {state} {sym} {sym}'
                output_lines.append(s)
                state += 1
            s = f'{state - 1} {end_state} {path[-1]} {path[-1]}'
            output_lines.append(s)
        elif len(path) == 1:
            s = f'{start_state} {end_state} {path[0]} {path[0]} {cost}'
            output_lines.append(s)
        elif len(path) == 2:
            s = f'{start_state} {state} {path[0]} {path[0]} {cost}'
            output_lines.append(s)
            state += 1
            s = f'{state - 1} {end_state} {path[1]} {path[1]}'
            output_lines.append(s)
        else:
            print(path)
            raise RuntimeError
    with open(outf, 'w') as fh:
        for line in output_lines:
            fh.write(f'{line}\n')
        fh.write(f'{end_state}\n')
    if write_syms_f:
        with open(write_syms_f, 'w') as fh:
            for sym, i in syms.items():
                fh.write(f'{sym} {i}\n')


def create_lca(syms, dct_isyms):
    fst = WrappedFst()
    start = fst.add_state()
    fst.set_start(start)
    state = start
    for sym in syms:
        newstate = fst.add_state()
        if not dct_isyms:
            sym = int(sym)
            fst.add_arc(state, newstate, sym, sym, 0.)
        else:
            if sym in dct_isyms:
                fst.add_arc(state, newstate, dct_isyms[sym], dct_isyms[sym], 0.)
            else:
                unkid = dct_isyms['<unk>']
                fst.add_arc(state, newstate, unkid, unkid, 0.)

        state = newstate
    fst.set_final(state)
    return fst


def create_one_lca(inf, outf, dct_isyms):
    syms = []
    with open(inf) as fh:
        for line in fh:
            if line.strip():
                syms.extend(line.split())

    fst = create_lca(syms, dct_isyms)
    fst.write(outf)


def create_ark_lca(inf, outf, dct_isyms):
    with open(inf) as fh:
        for line in fh:
            uttid, *syms = line.split()
            fst = create_lca(syms, dct_isyms)
            fst.write_ark_entry(uttid, outf)


def main(inf: "Path to file with input symbols",
         outf: "Path to fst file to create.",
         read_syms_f: ('', 'option', None),
         write_syms_f: ('', 'option', None),
         isark: ('', 'flag', None) = False,
         ismultiple: ('Multiple paths in FST', 'flag', None) = False,
         index_offset: ('', 'option', None, int) = 0,
         cost: ('', 'option', None, float) = 0.):
    """ Create linear chain automata
    Three modes:
        1. Creates a single LCA from a single sequence.
        2. Creates a "comb", multiple arc sequences from a single start and end state.
        3. Creates multiple LCAs from an ark file, one for each ark entry.
    """

    dct_isyms = {}
    if read_syms_f:
        with open(read_syms_f) as fh:
            for line in fh:
                w, i = line.split()
                i = int(i)
                dct_isyms[w] = i
                dct_isyms[i] = w

    if ismultiple:
        create_multiple_lca_txt(inf, outf, dct_isyms, write_syms_f, index_offset, cost)
    elif not isark:
        create_one_lca(inf, outf, dct_isyms)
    else:
        if os.path.isfile(outf): os.remove(outf)
        create_ark_lca(inf, outf, dct_isyms)


if __name__ == "__main__":
    import plac
    plac.call(main)
