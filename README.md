Still WIP

Point of the fst wrapper is that thanks to using pybind11 it's very easy to write extensions.

Will be various python scripts that use the wrapper to do convenient things.

# Usage

In lib you need a symlink to a copy of the pybind11 repository. Then modify `CMakeLists.txt` by setting the correct paths
to include/ (you might also have to include the openfst root dir) and where the libfst\* files are.

`mkdir build && cmake .. && make`

Then to install copy the `.so` to your sites directory, you can find it by calling `python -m site`.

To use your `LD_LIBRARY_PATH` needs to have the path to the openfst libraries.

```
from wrappedfst import WrappedFst

# do cool stuff, see bottom of lib/fst-wrapper.cc for available methods 
```
