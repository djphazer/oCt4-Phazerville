# o\_C and Teensy 4.0

And so it begins. Building on top of pld's [oCt4drv](https://github.com/patrickdowling/oCt4drv) drivers, I'm attempting to rewrite/port Hemisphere and friends to Teensy 4.0. There is still much to be done.

It may be a bit of a mess... possibly eligible for r/programminghorror. This will be my sandbox as I'm learning more advanced C++ language nuances.

## TODO
- [x] Basic Hemisphere UI & I/O Test App
- [ ] Calibration
- [ ] EEPROM
- [ ] Applet Switching
- [ ] App Menu / App Switcher
- [ ] MIDI Handling
- [ ] FreqMeasure for Tuner applet
- [ ] Port stock O_C Apps (maybe)

## Caveats
The ADC is apparently lower resolution than Teensy 3.2, so pitch CV input will be less accurate. This shouldn't matter too much when quantizing, unless you're using scales with sub-semitone increments. V/Oct tracking for oscillators like Ebb&LFO will be less reliable. Modulating input parameters may be noisier.

The Trigger inputs and DAC output are effectively the same. Sequencers, Envelopes, and other Modulation generators will work just fine.

## An Appeal

I am self-employed. My lifestyle costs around $100 per day.

If my work stands to benefit you, please consider reaching out to help cover expenses!
