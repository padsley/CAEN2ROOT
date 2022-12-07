# CAEN2ROOT
Converter for making .csv files from my Caen digitiser into ROOT TTrees

This is a converter for making the .csv files from my DT5560SE Caen digitiser into ROOT TTrees. Currently it only works for listmode data, not oscilloscope traces.

Usage:

root
.L CAEN2ROOT.cpp
CAEN2ROOT("input_file.csv","output_file.root",false);
The last flag is true = oscilloscope trace data, false = listmode data.

For the listmode conversion, the first step is to convert everything into a TTree of just raw channel-by-channel data (which I am calling "entries"), entirely independent of each other.

Once this is done, the TTree is then read back in and events are constructed based on (a) the number of entries (which is a variable set in the code) and (b) the time window (which is another variable set in the code).

This then makes a new TTree which is event-based rather than entry-based. It seems to work (for some data with a source) but the entry window might need some fiddling because the timestamps have a limited range due to the internal clock (I think) so they wrap around.
