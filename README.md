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
import pandas as pd

process = decoder_bindings.ProcessEvents(light_slot=14)

test_file = "/path/to/file/pGRAMS_bin_X.dat"
process.open_file(test_file)

# Iterate through file, one event at a time
# it will return True until the last event is processed
# in which case it will return False
light_fem = 16
event_num = 0
readout_data = []
while process.get_event():
    try:
        tmp_dict = process.get_event_dict()
        readout_data.append(tmp_dict)
        event_num += 1
    except:
        continue

readout_df = pd.DataFrame(readout_data)
```
Each row is an event with a dictionary of both the charge and light
data. Below is an example of an event.
(the charge event number is +1 to the real event number) 

```python
{'slot_number': {0: array([13, 14, 15, 16], dtype=uint16)},
 'num_adc_word': {0: array([48959, 48959, 48959,   121], dtype=uint32)},
 'event_number': {0: array([2, 2, 2, 1], dtype=uint16)},
 'event_frame_number': {0: array([71, 71, 71, 72], dtype=uint16)},
 'trigger_frame_number': {0: array([72, 72, 72, 72], dtype=uint16)},
 'check_sum': {0: array([40394, 41580, 18995, 20241], dtype=uint16)},
 'trigger_sample': {0: array([123, 123, 123, 123], dtype=uint16)},
 'light_channel': {0: array([2, 2, 2, 2, 2], dtype=uint16)},
 'light_frame_number': {0: array([71, 72, 72, 73, 74], dtype=uint32)},
 'light_readout_sample': {0: array([2268,  476, 6876, 5084, 3292], dtype=uint16)},
 'light_adc_words': {0: array([[2050, 2050, 2120, 2588, 3033, 3229, 3205, 3051, 2851, 2653, 2486,
                                2355, 2258, 2189, 2142, 2110, 2089, 2074, 2064, 2059],
                               [2049, 2048, 2133, 2613, 3047, 3232, 3200, 3042, 2841, 2645, 2479,
                                2350, 2254, 2187, 2139, 2108, 2087, 2073, 2063, 2058],
                               [2050, 2049, 2142, 2622, 3047, 3221, 3183, 3023, 2823, 2629, 2468,
                                2342, 2248, 2182, 2138, 2107, 2086, 2073, 2064, 2060],
                               [2049, 2049, 2158, 2645, 3060, 3225, 3180, 3017, 2815, 2623, 2462,
                                2338, 2247, 2180, 2136, 2106, 2086, 2071, 2064, 2057],
                               [2050, 2049, 2170, 2662, 3071, 3230, 3181, 3016, 2813, 2621, 2460,
                                2336, 2244, 2180, 2136, 2106, 2085, 2072, 2063, 2058]],
                              dtype=uint16)},
 'charge_channel': {0: array([  0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,
                                13,  14,  15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,
                                26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,
                                39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,
                                52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,
                                65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,
                                78,  79,  80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,
                                91,  92,  93,  94,  95,  96,  97,  98,  99, 100, 101, 102, 103,
                                104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116,
                                117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127, 128, 129,
                                130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142,
                                143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155,
                                156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168,
                                169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181,
                                182, 183, 184, 185, 186, 187, 188, 189, 190, 191], dtype=uint16)},
 'charge_adc_words': {0: array([[2044, 2044, 2044, ..., 2044, 2044, 2044],
                                [2042, 2041, 2042, ..., 2041, 2041, 2042],
                                [2043, 2044, 2043, ..., 2043, 2044, 2043],
                                ...,
                                [ 460,  460,  460, ...,  460,  460,  460],
                                [ 469,  470,  469, ...,  469,  470,  470],
                                [ 472,  473,  473, ...,  473,  472,  473]],
                               shape=(192, 763), dtype=uint16)}}
```

The full waveform for a channel in an event can be reconstructed 
using the `get_full_light_waveform()` and `get_full_light_axis()` 
and plotted. The arguments fo the functions is

```python
get_full_light_waveform(<channel_number>, <roi_sample>, <roi_frame>, <roi_list>)
```
```python
get_full_light_axis(<trigger_frame>, <trigger_sample>, <roi_frame>)
```

```python
event = 5
channel = 3
full_waveform = proc.get_full_light_waveform(channel, readout_df['light_channel'][event],
                                             readout_df['light_readout_sample'][event],
                                             readout_df['light_frame_number'][event],
                                             readout_df['light_adc_words'][event])

full_axis = proc.get_full_light_axis(readout_df['trigger_frame_number'][event][fem_number].astype(int),
                                     readout_df['trigger_sample'][event][fem_number].astype(int),
                                     readout_df['light_frame_number'][event][fem_number])

plt.figure(figsize=(16,4))
plt.plot(full_axis/1e3, full_waveform)
plt.xlabel("[$\mu$s]")
plt.show()
```