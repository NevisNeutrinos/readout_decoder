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
If you want to uninstall the bindings run,

```
pip uninstall decoder_bindings
```

Then the decoder can be used from python such as pandas

```python
import decoder_bindings
import pandas

process = decoder_bindings.ProcessEvents()

test_file = "/path/to/file/pGRAMS_bin_X.dat"
process.open_file(test_file)

# Iterate through file, one event at a time
# it will return False until the last event is processed
# in which case it will return True
event_num = 0
event_dict = {}
while process.get_event():
    event_dict[event_num] = process.get_event_dict()
    print(event_num)
    event_num += 1

df = pandas.DataFrame(event_dict).T
```
Each row is an event with `Charge` and `Light` columns,
```python
	Light	Charge
0	{16: {'slot_number': 16, 'num_adc_word': 121, ...	{0: {'slot_number': 0, 'num_adc_word': 0, 'eve...
1	{16: {'slot_number': 16, 'num_adc_word': 121, ...	{0: {'slot_number': 0, 'num_adc_word': 0, 'eve...
2	{16: {'slot_number': 16, 'num_adc_word': 121, ...	{0: {'slot_number': 0, 'num_adc_word': 0, 'eve...
3	{16: {'slot_number': 16, 'num_adc_word': 121, ...	{0: {'slot_number': 0, 'num_adc_word': 0, 'eve...
4	{16: {'slot_number': 16, 'num_adc_word': 121, ...	{0: {'slot_number': 0, 'num_adc_word': 0, 'eve...
```
and each event is a dictionary of the data. For the charge FEMs we have, 

```python
>>> event = 5
>>> fem_number = 15
>>> df['Charge'][event][fem_number]


{'slot_number': 15,
 'num_adc_word': 48959,
 'event_number': 2479415,
 'event_frame_number': 15248383,
 'trigger_frame_number': 44023,
 'check_sum': 13383543,
 'trigger_sample': 240,
 'channel': array([ 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
        17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
        34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
        51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63], dtype=uint16),
 'adc_words': array([[2043, 2043, 2043, ..., 2043, 2043, 2043],
        [2047, 2048, 2047, ..., 2047, 2048, 2048],
        [2041, 2041, 2041, ..., 2041, 2041, 2041],
        ...,
        [ 461,  460,  461, ...,  461,  460,  461],
        [ 469,  470,  470, ...,  470,  470,  469],
        [ 473,  472,  473, ...,  472,  472,  473]],
       shape=(64, 763), dtype=uint16)}
```

The light FEM has this data format. The `_reco` suffix is the light waveform that
has been reconstructed from the individual ROIs within the event.

```python
>>> event = 5
>>> fem_number = 16
>>> df['Light'][event][fem_number]


{'slot_number': 16,
 'num_adc_word': 121,
 'event_number': 2488850,
 'event_frame_number': 15224291,
 'trigger_frame_number': 19931,
 'check_sum': 5198732,
 'trigger_sample': 90,
 'channel': array([3, 3, 3, 3, 3], dtype=uint16),
 'light_frame_number': array([10, 11, 11, 12, 13], dtype=uint16),
 'light_readout_sample': array([2207,  415, 6815, 5023, 3231], dtype=uint16),
 'adc_words': array([[2047, 2047, 2121, 2540, 2979, 3207, 3221, 3094, 2905, 2708, 2535,
                      2393, 2287, 2209, 2154, 2117, 2092, 2075, 2065, 2058],
                     [2047, 2047, 2133, 2564, 2995, 3211, 3217, 3086, 2895, 2699, 2526,
                      2388, 2282, 2206, 2153, 2116, 2091, 2075, 2064, 2058],
                     [2047, 2047, 2146, 2585, 3009, 3216, 3214, 3079, 2887, 2692, 2520,
                      2382, 2279, 2203, 2151, 2115, 2091, 2074, 2064, 2058],
                     [2047, 2046, 2158, 2603, 3020, 3218, 3211, 3072, 2880, 2684, 2514,
                      2377, 2274, 2200, 2148, 2113, 2089, 2073, 2063, 2057],
                     [2046, 2047, 2171, 2623, 3035, 3222, 3207, 3064, 2870, 2676, 2507,
                      2373, 2271, 2198, 2147, 2112, 2089, 2073, 2063, 2057]],
                    dtype=uint16),
 'adc_words_reco': array([[2048, 2048, 2048, ..., 2048, 2048, 2048],
                          [2048, 2048, 2048, ..., 2048, 2048, 2048],
                          [2048, 2048, 2048, ..., 2048, 2048, 2048],
                          ...,
                          [2048, 2048, 2048, ..., 2048, 2048, 2048],
                          [2048, 2048, 2048, ..., 2048, 2048, 2048],
                          [2048, 2048, 2048, ..., 2048, 2048, 2048]],
                         shape=(32, 81600), dtype=uint16),
 'adc_axis_reco': array([-812515.625, -812500.   , -812484.375, ...,  462437.5  ,
                         462453.125,  462468.75 ], shape=(81600,))}
```