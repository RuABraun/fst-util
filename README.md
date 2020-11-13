Still WIP

Point of the fst wrapper is that thanks to using pybind11 it's very easy to write extensions.

Will be various python scripts that use the wrapper to do convenient things.

# Usage

In lib you need a symlink to a copy of the pybind11 repository.

To use your `LD_LIBRARY_PATH` needs to have the path to the openfst libraries.

```
from wrappedfst import WrappedFst
# do cool stuff
```
