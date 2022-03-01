import plac
from wrappedfst import WrappedFst
import re

def main(inf, symtab_f, outf, count=100000):
    fst = WrappedFst(inf)

    symtab = {}
    with open(symtab_f) as fh:
        for line in fh:
            word, i = line.split('\t')
            i=int(i)
            symtab[i] = word

    lines = fst.randpath(count)
    with open(outf, 'w') as fh:
        for labels in lines:
            words = [symtab[i] for i in labels]
            words = ' '.join(words)
            words = re.sub('\s+', ' ', words)
            fh.write(f'{words}\n')


plac.call(main)
