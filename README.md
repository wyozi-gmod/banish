# Banish

A decentralized ban synchronization addon for Garry's Mod.

## Structure
`banish-rs` contains the Rust library for HTTP stuff
`gmod-binary` contains the C++ binary module that is the bridge between Lua and Rust.
`gmod-addon` contains the Garry's Mod addon

## Compiling

### Compiling Rust library
On Windows use `cargo build --target=i686-pc-windows-msvc`. Otherwise just `cargo build`.

### Compiling GMod binary module
Run `BuildProjects.bat`, compile the project in the folder for your platform. Some notes:
- In Visual Studio, the banish.lib path might be wrong. To fix it, edit linker input and add one more `..\`.
- You need to remove `include <stdbool.h>` line from banish-rs/include/banish.h, as VS doesn't support C99.