import decoder_bindings

process = decoder_bindings.ProcessEvents(light_slot=16)

test_file = "/Users/jonsensenig/work/grams/daq_ana/jon_test/pGRAMS_bin_992.dat"
process.open_file(test_file)

# Can either set the number of events to read
# if the end of file is reached before number of
# requested events, then it will just stop and close file
num_events = 5
process.get_num_events(num_events)

# ..OR iterate through file, one event at a time
# it will return False until the last event is processed
# in which case it will return True
event_num = 0
while process.get_event():
    print(event_num)
    event_num += 1
