# Pro-800 Manager Plugin

Plugin (VST / AU) and standalone application for remote controlling the Behringer Pro-800 synthesizer and managing the saved presets (import/export/move/rename).

The code is platform-independent and should work for Windows/Linux/MacOS without changes.

## Related works
 - JUCE Library:

   https://juce.com

 - Pamplejuice:

   https://github.com/sudara/pamplejuce, used as basis for the build environment with cmake

 - Behringer Pro-800 Preset Structure:

   https://cdn.mediavalet.com/aunsw/musictribe/q3-BVIpZck-zi_hAUX2V2Q/McqR_H7pVkW5V21CKYq9Pg/Original/Preset-Structure-Document_BE_0718-ABF_PRO800.pdf

## Documentation
 - [Pro-800 CC Messages](docs/Pro800CCMessages.md)
 - [Pro-800 SysEx Messages](docs/Pro800SysExMessages.md)

## Known Limitations
 - Binaries are currently not signed
 - Most parameter values have the range [0, 65535] internally in the synth and in the presets (even if only the range [0-999] is displayed). CC values only cover the range [0,127], so getting / setting values will be at a lower granularity.
 - Setting the VCA/VCF envelope speeds is not working properly because of a bug in the Pro-800 firmware that is hard to work around. The CC values for setting the speed externally are the opposite to what they are when changed on the synth directly. See also the corresponding forum thread: https://community.musictribe.com/discussion/behringer/96/109/330871/pro-800-cc-values-for-vcf-and-vca-speeds---firmware-bug
 - I have not found a way to get the complete current state from the synth. Easiest workaround: Load a preset from the software, it will apply the current settings to all the UI elements.
 - The UI is... historically grown... and I am not a UI/UX designer. For the moment it serves its purpose
 - Testing on different platforms is minimal. I'm mainly working on MacOS, so that will most likely work best. Windows/Linux builds are provided by autmated GitHub workflows and are not guaranteed to be tested.

## Building
1. Clone the repository

1. Initialize and update the submodules (including the JUCE library dependencies):
    
       git submodule update --init --recursive

1. Run the build

       cmake .
       make
