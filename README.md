# ft_nm

`ft_nm` is a school project that reimplements the standard Unix command `nm`. It displays the symbol table of binary files (ELF format), supporting both x86_64 and x86_32 architectures.

## Description

The `nm` command lists symbols from object files. This implementation mimics the system `nm` behavior, parsing ELF files to extract and display symbol information such as addresses, types (e.g., T for text, D for data, U for undefined), and names.

It is designed to deepen understanding of:
- The ELF (Executable and Linkable Format) file structure.
- Memory mapping (`mmap`) for efficient file access.
- Endianness handling (supporting both Little and Big Endian).
- Bit manipulation and C programming internals.

## Features

- **ELF Support:** Handles 64-bit and 32-bit ELF binaries.
- **Symbol Types:** Correctly identifies and displays standard symbol types (T, t, D, d, B, b, U, W, w, etc.).
- **Endianness:** Automatically detects and handles file endianness.
- **Bonus Flags:** Supports several standard `nm` flags:
  - `-a`: Display all symbols, including debugger-only symbols.
  - `-g`: Display only external symbols.
  - `-u`: Display only undefined symbols.
  - `-r`: Reverse the sort order.
  - `-p`: Do not sort the symbols; display them in the order encountered.

## Compilation

The project includes a `Makefile` for easy compilation.

```bash
make
```

This will generate the `ft_nm` executable.

## Usage

```bash
./ft_nm [options] <file...>
```

### Examples

Basic usage:
```bash
./ft_nm /bin/ls
```

Using flags:
```bash
./ft_nm -a -r my_program
```

Displaying external symbols only:
```bash
./ft_nm -g library.so
```

## Testing

A test suite is included to verify correctness against the system `nm`.

### Test Files
The `test/` directory contains various test files, including:
- Valid 32-bit and 64-bit binaries.
- Corrupted or invalid files (to test error handling).
- Files with specific edge cases (e.g., weak symbols).

### Running Tests
A script `test.sh` is provided to automate testing. It compares the output of `ft_nm` with the system `nm` for all files in the `test/` directory.

```bash
./test.sh
```

The script will output `OK` for matches and `KO` (with a diff) for discrepancies. It runs standard tests and a second pass with bonus flags (`-ragpur`).

**Note:** The test script filters out system-specific "bfd plugin" warnings to ensure accurate comparison.

## Author

This project was developed as part of a computer science curriculum.
