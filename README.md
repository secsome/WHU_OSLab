# WHU_OSLab
武汉大学信息安全操作系统实验

## Requirements:
- `Ubuntu` >= 20.04 (other releases not tested)
- `gcc`
- `gdb`
- `nasm` >= 2.15.5
- `Makefile`
- `Bochs` with gdb-stub support
- `VSCode` Extensions:
  - `asm-enable-debug`
  - `C/C++`
  - `C/C++ Extension Pack`

## Usage:
1. - In your linux terminal:
      1. `git clone https://github.com/secsome/WHU_OSLab.git`
      2. `cd WHU_OSLab`
      3. `vscode`
   - Or just open this workdir In `Visual Studio Code` manually
2. Press `F5` to launch the simulation. `Ctrl + Shift + B` for `make image`.

## Features:
- Full source code debug, both .asm and .c files are supported
- Rearranged code structure
- One-click deployment

## Footnotes:
- This project is based on the cdimage provided by `OrangeOS`.
- Still **WIP** while the author hasn't finish the labs right now.

## Known-bugs:
- If compile failed while using `F5`, the `VSCode` debugger may get stuck, then you have to restart `VSCode`. So mkae sure you can pass compile before doing so, you can check that by pressing `Ctrl + Shift + B`.