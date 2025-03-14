To install the python bindings, make sure you have, 
* python
* pip
* scikit-build

Then in this directory just run
```
pip install .
```
Now the bindings should be available in python, e.g.
```python
>>> import decoder_bindings
>>> proc = decoder_bindings.ProcessEvents()
```
where `proc` is now an isntance of the ProcessEvents class
and you can open files and decode events. See the 
`python/` directory for an example.