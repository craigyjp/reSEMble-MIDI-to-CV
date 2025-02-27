# MIDI to CV for the reSEMble monosynth.

This MIDI to CV based on the RP2040 Waveshare Zero board is a 3 note polyphonic (paraphonic) MIDI to CV.

In standard mode the MIDI to CV is unison, all 3 CV outputs are identical, with a detune option that slightly detunes VCO2 & more on VCO3.

Polymode will enable a dynamic 3 note paraphonic synth where pressing more than 1 key will switch the second and third notes to the pitch that has been requested. When a key is released it moves back to the primary note.

There is an onboard LFO based on the Electric Druid TapLFO3-D chip. 
This is a voltage controlled LFO capable of 16 waveforms and and is merged into the CV lines based on your settings for FM/AT depth.

The interface also responds to pitch bend and the range can be set from 0-12 semitones.

Aftertouch depth of the LFO to the FM

Mod Wheel depth of the LFO to the FM.

Gate and trigger outputs are +5V

LFO MIDI Sync option.

Top, Bottom, Last note priority in mono mode.

Digital glide is also included in mono mode.

All parameters are saved 10 seconds after the last edit and reloaded on power up.

MIDI CC can also be used to control functions thus allowing it to inetgrate into a DAW.

* MIDI CC messages
  
CC05 Portamento time 0-127, 0-10 Seconds

CC15 Detune VCO2 & 3 0-127

CC16 Bendwheel Range 0-127,  0-12 semitones

CC17 Modwheel Depth FM 0-127

CC19 AT Depth FM 0-127

CC20 LFO Rate 0-127

CC21 LFO Wave 0-127 (waveform 1-8)

CC22 LFO Multiplier 0-127 (0.5 to 3.0)

CC23 LFO alternative waves 0-127, original/alternative

CC65 Portamento Off/On 0-127, 0=Off, 127=On 

CC123 AllNotesOff 127

CC127 Note Priority 0-127, Top, Bottom, Last.

