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

process = decoder_bindings.ProcessEvents()

test_file = "/path/to/file/pGRAMS_bin_X.dat"
process.open_file(test_file)

# Iterate through file, one event at a time
# it will return True until the last event is processed
# in which case it will return False
light_fem = 16
event_num = 0
charge_data = []
light_data = []
while process.get_event():
    try:
        tmp_dict = process.get_event_dict()
        charge_data.append(tmp_dict['Charge'])
        light_data.append(tmp_dict['Light'][light_fem])
        print(event_num)
        event_num += 1
    except:
        continue

charge_df = pd.DataFrame(charge_data)
light_df = pd.DataFrame(light_data)
```
Each row is an event with a dictionary of the data. For the charge FEMs we have, 
(the event number is +1 to the real event number) 

```python
{'slot_number': 15,
 'num_adc_word': 38207,
 'event_number': 6,
 'event_frame_number': 5931,
 'trigger_frame_number': 5923,
 'check_sum': 7751783,
 'trigger_sample': 94,
 'channel': array([ 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15, 16,
                    17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33,
                    34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50,
                    51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63], dtype=uint16),
 'adc_words': array([[2045, 2044, 2045, ..., 2045, 2045, 2045],
                     [2050, 2050, 2050, ..., 2050, 2050, 2050],
                     [2045, 2045, 2044, ..., 2045, 2044, 2045],
                     ...,
                     [   0,    0,    0, ...,    0,    0,    0],
                     [   0,    0,    0, ...,    0,    0,    0],
                     [   0,    0,    0, ...,    0,    0,    0]],
                    shape=(64, 595), dtype=uint16)}
```

The light FEM has this data format. 

```python
{'slot_number': 16,
 'num_adc_word': 121,
 'event_number': 5,
 'event_frame_number': 316,
 'trigger_frame_number': 316,
 'check_sum': 5198182,
 'trigger_sample': 130,
 'channel': array([3, 3, 3, 3, 3], dtype=uint16),
 'light_frame_number': array([315, 316, 317, 318, 318], dtype=uint16),
 'light_readout_sample': array([5940, 4148, 2356,  564, 6964], dtype=uint16),
 'adc_words': array([[2046, 2047, 2116, 2541, 3004, 3251, 3266, 3131, 2932, 2727, 2545,
                      2399, 2291, 2211, 2156, 2118, 2093, 2076, 2065, 2058],
                     [2047, 2046, 2124, 2556, 3014, 3252, 3261, 3124, 2923, 2717, 2537,
                      2395, 2287, 2209, 2154, 2117, 2093, 2076, 2065, 2059],
                     [2047, 2047, 2138, 2581, 3032, 3257, 3257, 3114, 2912, 2709, 2530,
                      2388, 2282, 2205, 2152, 2116, 2092, 2076, 2065, 2058],
                     [2047, 2047, 2148, 2598, 3043, 3260, 3254, 3107, 2905, 2701, 2525,
                      2385, 2280, 2203, 2151, 2114, 2091, 2074, 2064, 2058],
                     [2047, 2047, 2160, 2618, 3055, 3263, 3248, 3099, 2897, 2695, 2518,
                      2380, 2277, 2201, 2150, 2113, 2092, 2074, 2065, 2057]],
                    dtype=uint16)}
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
full_waveform = proc.get_full_light_waveform(channel, light_df['channel'][event],
                                             light_df['light_readout_sample'][event],
                                             light_df['light_frame_number'][event],
                                             light_df['adc_words'][event])

full_axis = proc.get_full_light_axis(light_df['trigger_frame_number'][event],
                                     light_df['trigger_sample'][event],
                                     light_df['light_frame_number'][event])
plt.plot(full_axis/1e3, full_waveform)
plt.xlabel("[$\mu$s]")
```