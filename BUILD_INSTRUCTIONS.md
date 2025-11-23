# Build Instructions

## Quick Start

### Linux/macOS

```bash
# Using Make
make
./build/bin/civilization --new

# Using CMake
mkdir build && cd build
cmake ..
make
./civilization --new
```

### Windows

#### Using Visual Studio

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 16 2019"
cmake --build . --config Release
Release\civilization.exe --new
```

#### Using MinGW

```cmd
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
build\bin\civilization.exe --new
```

## Build Options

### Debug Build

```bash
# Make
make debug

# CMake
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```

### Release Build

```bash
# Make
make release

# CMake
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```

### Clean Build

```bash
# Make
make clean

# CMake
rm -rf build
```

## Requirements

- C11 compatible compiler (GCC 4.9+, Clang 3.3+, MSVC 2015+)
- CMake 3.10+ (optional)
- Make (optional, for Makefile build)

## Troubleshooting

### Compilation Errors

1. **Missing includes**: Ensure all header files are in the `include/` directory
2. **Linker errors**: Make sure math library is linked (`-lm` flag)
3. **Platform-specific**: Check that platform-specific code (Windows/Linux) is properly guarded

### Runtime Errors

1. **Segmentation fault**: Check for null pointer dereferences
2. **Memory leaks**: Use valgrind (Linux) or AddressSanitizer
3. **File I/O errors**: Check file permissions and paths

## Testing

```bash
# Run benchmark
./build/bin/civilization --benchmark

# Test save/load
./build/bin/civilization --new
# (let it run, then Ctrl+C)
# Check for autosave files
./build/bin/civilization --load autosave_*.json
```

## Installation

### System-wide Installation (Unix-like)

```bash
make install
```

### Custom Installation

```bash
# CMake
cmake -DCMAKE_INSTALL_PREFIX=/custom/path ..
make install

# Make
PREFIX=/custom/path make install
```

