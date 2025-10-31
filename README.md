# hackerultralib

Configurable modern N64 operating system, designed for game modding.

Based on [ultralib](https://github.com/decompals/ultralib/).

This repo builds `libgultra.a`, `libgultra_d.a`, or `libgultra_rom.a`, with modern MIPS GCC.

## Features
 - Latest revision of libultra
 - Built-in GC controller support
 - Button remapping support
 - Various performance improvements

## Build dependencies

The build process requires the following packages:

- build-essential
- python3
- binutils-mips-linux-gnu
- gcc-mips-linux-gnu

Under Debian / Ubunutu you can install them with the following commands:

```bash
sudo apt update
sudo apt install build-essential python3 binutils-mips-linux-gnu gcc-mips-linux-gnu
```

For other distros, refer to the HackerSM64 [Installing Dependencies](https://github.com/HackerN64/HackerSM64/wiki/Installing-Dependencies) page, since package names will differ.

## Building
Simply run `make`, and the final library will be at `build/libgultra_rom.a`.

For building other targets, set the `TARGET` variable in the command:
- `make TARGET=libgultra_d`

## Integrate your Decomp
To integrate hackerlibultra in a new modding-friendly decomp repo, there are a few steps:

1. Have all your libultra functions split out and labeled.
2. Copy the built library from this repo to yours (or make it a submodule/subtree)
3. Use the headers in this repo's `include/PR`, as new functions and modified structs have been introduced.
4. Make sure the game builds with all the changes

