# Pro-800 Manager Plugin

Plugin (VST / AU) and standalone application for remote controlling the Behringer Pro-800 synthesizer and managing the saved presets (import/export/move/rename).

The code is platform-independent and should work for Windows/Linux/MacOS without changes.

## Related works



## Building
1. Clone the repository

1. Initialize and update the submodules (including the JUCE library dependencies):
    
       git submodule update --init --recursive

1. Run the build

       cmake .
       make