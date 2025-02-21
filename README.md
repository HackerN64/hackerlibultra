# hackerultralib

Configurable modern N64 operating system, designed for game modding.

Based on [ultralib](https://github.com/decompals/ultralib/).

This repo builds `libgultra.a`, `libgultra_d.a`, or `libgultra_rom.a`, with modern MIPS GCC.

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
