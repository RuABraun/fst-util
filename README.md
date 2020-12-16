Still WIP

Point of the fst wrapper is that thanks to using pybind11 it's very easy to write extensions.

Will be various python scripts that use the wrapper to do convenient things.

# Usage

In lib you need a symlink to a copy of the pybind11 repository.

`mkdir build && cmake .. && make`

Then to install copy the `.so` to your sites directory, you can find it by calling `python -m site`.

To use your `LD_LIBRARY_PATH` needs to have the path to the openfst libraries.

```
from wrappedfst import WrappedFst
# do cool stuff
```
