# Pro-800 Manager Plugin

Plugin (VST3 / AU / AUv3) and standalone application for remote controlling the Behringer Pro-800 synthesizer and managing the saved presets (import/export/move/rename).

The code is platform-independent and should work for Windows/Linux/MacOS without changes.

## Related works
 - JUCE Library:

   https://juce.com

 - Pamplejuice:

   https://github.com/sudara/pamplejuce, used as basis for the build environment with cmake

 - Behringer Pro-800 Preset Structure:

   https://cdn.mediavalet.com/aunsw/musictribe/q3-BVIpZck-zi_hAUX2V2Q/McqR_H7pVkW5V21CKYq9Pg/Original/Preset-Structure-Document_BE_0718-ABF_PRO800.pdf

 - The reverse-engineering notes the protocol documentation in `docs/` is distilled from (bench sessions, captures, a Python CLI):

   https://github.com/steinachim/reverse_engineering (folder `pro800/`)

## Documentation
 - [Pro-800 CC Messages](docs/Pro800CCMessages.md)
 - [Pro-800 SysEx Messages](docs/Pro800SysExMessages.md)

## Known Limitations
 - Binaries are currently not signed
 - Most parameter values have the range [0, 65535] internally in the synth and in the presets (even if only the range [0-999] is displayed). CC values only cover the range [0,127], so getting / setting values will be at a lower granularity.
 - Setting the VCA/VCF envelope speeds is not working properly because of a bug in the Pro-800 firmware that is hard to work around. The CC values for setting the speed externally are the opposite to what they are when changed on the synth directly. See also the corresponding forum thread: https://community.musictribe.com/discussion/behringer/96/109/330871/pro-800-cc-values-for-vcf-and-vca-speeds---firmware-bug
 - The synth cannot report the sound it is currently playing (its edit buffer). What it can report is the *stored* preset records, the preset it is on, its settings, and the physical positions of its knobs and switches - which are not the sound, because loading a preset does not move them. The plugin therefore shows in its status line what its controls currently represent (`B05 as stored`, `+ edits`, `aligned with the panel`, `unknown`), offers Load and Revert to put the controls and the synth on a known, stored preset, and "Align with panel" in the same status line, which reads where the synth's knobs and switches physically sit, shows them and sends them back as CC so that what you hear is what the panel says. Controls whose value has not come from anywhere yet are dimmed.
 - The MIDI channel is read from the synth after connecting (including the rear DIP switches); pick one by hand only if that fails. If the synth's own MIDI channel is set to OFF it ignores notes and CC, and the plugin says so.
 - The UI is... historically grown... and I am not a UI/UX designer. For the moment it serves its purpose
 - Testing on different platforms is minimal. I'm mainly working on MacOS, so that will most likely work best. Windows/Linux builds are provided by autmated GitHub workflows and are not guaranteed to be tested.

## Building
1. Clone the repository

1. Initialize and update the submodules (including the JUCE library dependencies):
    
       git submodule update --init --recursive

1. Configure and build (out of source; the first configure also downloads Catch2 for the tests)

       cmake -B build -DCMAKE_BUILD_TYPE=Release
       cmake --build build

1. Run the unit tests (they cover the MIDI protocol layer in `source/midi`)

       ctest --test-dir build --output-on-failure

## Contributing
 - The code is formatted with `clang-format` (see `.clang-format`); CI checks it. To format locally with the same version:

       pip install clang-format==22.1.8
       clang-format -i $(git ls-files 'source/*.cpp' 'source/*.h' 'tests/*.cpp' 'tests/*.h')

   Hand-aligned tables are wrapped in `// clang-format off` / `on`.
 - The MIDI protocol is documented in [docs/](docs/); the byte tables there must match `source/tailoring/`.
