# readout_decoder

To install the decoder you need:
* cmake >= 3.30
* C++17 or newer
* python >= 3.10
* scikit-build

Then git clone the package to your desire location. Then to include 
`pybind11` for the python bindings, from the root directory run

```
git submodule init && git submodule update
```

Then build the decoder with,

```
cd extern
pip install .
```