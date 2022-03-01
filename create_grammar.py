import plac
import itertools
from wrappedfst import WrappedFst


def updatesym(symtab, label):
    if label not in symtab:
        symtab[label] = len(symtab)
    return symtab[label]

def main(inf, symtab_f, outf):
    """ inf is a file which contains the different possible labels
    it looks like
    START
    a
    b

    x
    y
    END
    lines with no space in between them are understood as different options.
    A space means signifies the end of the different options, and the next
    "paragraph" would be arcs to the next state.
    TODO: improve description
    """
    symtab = {'<eps>': 0}

    fst = WrappedFst()
    state = fst.add_state()
    fst.set_start(state)
    nstate = fst.add_state()
    opts = []
    for line in open(inf):
        label = line.strip()
        if not label:
            state = nstate
            nstate = fst.add_state()
            continue
        if '(' in line and ')' in line:
            si = label.find('(')
            ei = label.find(')')
            prefixline = label[:si]
            suffixline = label[ei+1:]
            opts = label[si+1: ei]
            opts = opts.split(';')

            tstate = fst.add_state()
            n = updatesym(symtab, prefixline)
            fst.add_arc(state, tstate, n, n, 0.)
            estate = fst.add_state()
            for opt in opts:
                n = updatesym(symtab, opt)
                fst.add_arc(tstate, estate, n, n, 0.)
            n = updatesym(symtab, suffixline)
            fst.add_arc(estate, nstate, n, n, 0.)
        else:
            n = updatesym(symtab, label)
            fst.add_arc(state, nstate, n, n, 0.)
    fst.set_final(nstate)

    fst.determinize()
    fst.minimize()
    with open(symtab_f, 'w') as fh:
        for k, i in symtab.items():
            fh.write(f'{k}\t{i}\n')
    fst.write(outf)


plac.call(main)
