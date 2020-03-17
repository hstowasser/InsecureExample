#!/usr/bin/env python3
import wave
original_file = 'test.wav'
dout_file = 'test.drm.dout'
true_song = wave.open(original_file, 'r')
true_samples = true_song.readframes(true_song.getnframes())
try:
    # try to interpret as wav file
    dout_song = wave.open(dout_file, 'r')
    dout_samples = dout_song.readframes(dout_song.getnframes())
except wave.Error:
    # otherwise intepret as a file with raw samples
    with open(dout_file, 'rb') as f:
        dout_samples = f.read()
assert(dout_samples == true_samples)
