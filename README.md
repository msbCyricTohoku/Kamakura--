# Kamakura--

Kamakura-- is a minimal Qt-based editor designed for working with PHITS input files.
All C++ sources now live in the `src/` directory with icons and language files in
`src/resources`.

## Building
Install the Qt development tools (e.g. `qtbase5-dev`, `qt5-qmake`, `cmake`) and run:

```bash
qmake
make
```

## Features
- Tabbed editing interface
- Syntax highlighting for PHITS and Python
- Search and replace dialog
- Live document metrics in the status bar
- Save and Save As functions

After compilation, run the generated binary and open your `.i` or `.inp` scripts.

Enjoy!

Developed by Mehrdad S. Beni and Hiroshi Watabe, CYRIC, Tohoku University, 2023
