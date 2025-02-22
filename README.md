# MIDI to CV for the reSEMble monosynth.

This MIDI to CV based on the RP2040 Waveshare Zero board is a 3 note polyphonic MIDI to CV.

In standard mode the MIDI to CV is unison, all 3 CV outputs are identical, with a detune option that slightly detunes VCO2 & more on VCO3.

Polymode will enable a dynamic 3 note paraphonic synth where pressing more than 1 key will switch the second and third notes to the pitch that has been requested.

There is an onboard LFO based on the Electric Druid TapLFO3-D chip. 
This is a voltage controlled LFO capable of 16 waveforms and and is merged into the CV lines based on your settings for FM/AT depth.

The interface also responds to pitch bend and the range can be set from 0-12 semitones.

Gate and trigger outputs are +5V

Digital glide is also included in mono mode.

All parameters are saved 10 seconds after the last edit and reloaded on power up.

MIDI CC can also be used to control functions thus allowing it to inetgrate into a DAW.

