# Changelog

## Next Release

## 2.0

### Connecting
- Connect identifies the synth by its firmware version and reads its settings; the top bar shows the connection and warns about an unsupported firmware.
- The Send and Receive MIDI channels default to Auto: taken from the synth's MIDI RX / TX Channel settings, including the rear DIP switches. OFF and invalid channel values are called out, with a jump to the Settings tab. A channel can still be chosen by hand.
- The Pro-800's USB port is pre-selected when it is plugged in.
- A synth that stops answering ends the connection with a message instead of being polled forever.

### Presets and the sound
- A status line on every tab shows the preset the synth is on (followed every 1.5 s, also when it is selected on the synth) and what the Front Panel / Performance controls currently represent: `B05 as stored`, `+ edits`, `aligned with the panel`, `unknown`. Controls whose value has not come from anywhere yet are dimmed.
- Load selects a preset through the synth's settings block - no MIDI channel needed - and confirms the selection by reading it back before making the synth recall it. It warns before discarding edits the plugin knows of.
- Revert makes the synth reload the stored preset it is on.
- "Align with panel" reads where the synth's knobs and switches physically sit, shows them and sends them back as CC, so that what you hear is what the panel says.
- The synth's current preset is marked in the program list.

### Program management
- Dumps and transfers show their progress and can be cancelled.
- Empty slots are recognised and never sent or exported; a lost reply is re-sent; every program written to the synth is verified by reading it back, and slots that could not be confirmed are named.
- "All <<" asks for confirmation; imports are validated; presets are copied between the lists, so renaming one does not change the other.
- Presets stored by an older firmware (versions 109 and 110) get the values the synth itself uses for the newer fields when they are written back - pitch bend range 12 semitones, key tracking reference C4 - instead of zeros.
- Drag and drop within the local list: onto an occupied slot the two presets swap, onto an empty slot the preset is copied.

### Settings tab
- Every change is written as one block write and verified by reading it back: "Saving ..." while it lasts, and a value the synth did not take is undone and reported (a settings menu open on the synth writes its own value back).
- Settings changed on the synth's front panel are pointed out.
- A read-only Current Preset group; the channel combos name the channel the DIP switches select and show an out-of-range value.
- Factory Reset and Refresh are disabled while no synth is connected.

### Advanced tab
- The log shows the raw SysEx bytes above the decoded message, stays responsive at any length (capped at 20000 lines) and can hide the routine polling.
- Messages the Pro-800 must never receive (factory reset, bootloader, ...) are refused unless "Allow hazardous messages" is ticked, with a second confirmation for the irreversible ones.
- New prepared messages: panel read, knob read and write, reload, device name, product code, button presses with the full code list, panel lock, button lights, pitch bend range.
- "Read Panel to Log" prints every panel index and knob position decoded, and the MIDI channel the DIP switches select.

### Fixes
- Vibrato Amount showed the LFO Amount when a preset was loaded.
- A CC received from the synth could set a control from a stale value.
- The Pitch Bend Range slider showed one semitone too few for most presets (a 12-semitone preset as 11): the stored value's top five bits are the semitone count, and the synth writes different low bits depending on whether the value came from a SysEx message, a CC or the front panel. The docs describe the field as measured.

### Other
- New application icon.
- The window is resizable within the limits the layout can handle.
- Built on JUCE 9.0.2; macOS 10.14 or newer.
- The installers show the project's licence (AGPL-3.0) and a real readme instead of the build template's EULA and placeholder text.
- The protocol documentation in `docs/` covers everything the reverse-engineering sessions found: the control-surface and knob reads, the settings-write messages, the hazards, the transport behaviour, the settings block's selection pointer, how the synth converts older presets.
- Unit tests for the MIDI protocol layer and the session (`ctest`).

## 1.0

Initial release.
