# tmdd

A simple program to display PSX TMD (plus optional TIM, VDF & DAT) files; hence the name 'tmdd' (TMD display).

Usage:
```
    Usage: tmdd -t <TMD file> [-i <TIM files>...] [-v <VDF file>] [-d <DAT file>]
        -t <TMD file>      : Path to the TMD geometry file.
        -i <TIM files>...  : All associated TIM texture files. If none are passed,
                            the model will be displayed in wireframe mode.
        -v <VDF file>      : Path to a VDF mime file (optional).
        -d <DAT file>      : Path to a DAT animation file (optional).
                            This file is exclusively present in Parappa the Rapper &
                            Um Jammer Lammy.
```

To build, simply run `make`.

Building has not been tested on Windows & Linux (yet).

Some TMDs are bound to be able to not display properly; if so, feel free to submit an issue.

conhlee 2025
