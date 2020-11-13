import plac
from wrappedfst import WrappedFst
import math as m

def main(ints_f: 'File with integer ID of keywords on each line', weight: ('Weight of keyword / nonkeyword', 'positional', None, float), unk_id: ('ID of unk, will get higher cost', 'positional', None, int), outf):
    fst = WrappedFst()
    state = fst.add_state()
    fst.set_final(state)
    fst.set_start(state)
    offset = 2.
    keyword_cost = -m.log(weight) + offset
    unk_cost = -m.log(1-weight) + offset
    for n in open(ints_f):
        n = int(n)
        if n != unk_id:
            fst.add_arc(state, state, n, n, keyword_cost)
        else:
            fst.add_arc(state, state, n, n, unk_cost)

    fst.write(outf)

plac.call(main)
