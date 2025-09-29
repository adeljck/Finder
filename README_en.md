## finder Usage Guide

<p align="center">
  <img src="assets/logo.svg" alt="finder logo" width="420" />
</p>

[简体中文](README.md) | [English]

A multithreaded Windows command-line tool that scans file and directory names for keywords and outputs UTF-8 CSV.

### Build

- PowerShell (recommended):
```
./build.ps1            # default x64
./build.ps1 -Arch x86  # 32-bit
./build.ps1 -Clean     # clean artifacts
```

- Native Tools Command Prompt (cmd):
```
build.cmd              # default x64
build.cmd -Arch x86    # 32-bit
build.cmd -Clean       # clean artifacts
```

The scripts perform a privacy-friendly, reproducible Release build and remove intermediates, keeping only `finder.exe`.

### Basic Usage

```
finder.exe [options] [PATH1 PATH2 ...]
```

- If no paths are provided and `-all` is not used, usage help is printed and the program exits.
- With `-all`, the program enumerates all available drives (fixed/removable) and scans from their roots.

### Options

- `-t <N>`: Number of threads. Default is the number of logical processors (>=1).
- `-o <file>`: Write results to the specified file (UTF-8, no BOM). If not set, a timestamped file `result_YYYYMMDD_HHMMSS.csv` is created in the current directory.
- `-norec`: Non-recursive. Only scan the current directory level.
- `-followsymlink`: Follow symbolic links/reparse points when recursing (off by default to avoid loops).
- `-k <kw1,kw2,...>`: Comma-separated keywords (case-insensitive). If not specified, defaults to `vpn,password,passwd,pwd,account,账户,密码`.
- `-all`: Scan all available drives (fixed/removable) without specifying paths.
- `--debug`: Print every CSV hit to the console (by default only the total count is printed).
- `--debug-denied`: When access is denied for a directory, print `DENIED: <path>` to the console.

### Output and Logging

- Default behavior:
  - Does not print each hit to the console; prints only a final summary line, e.g., `Total matches: 123`.
  - If `-o` is not used, results are written to `result_YYYYMMDD_HHMMSS.csv` (UTF-8, no BOM) in the current directory.
- Debug mode:
  - With `--debug`, each hit is also printed to the console in the same CSV format.
- End-of-run summary:
  - Prints `Output file: <path>`, `Total matches: N`, and `Keywords: kw1,kw2,...`.

### CSV Format

- Each line has three fields:
  1) `fullpath` (e.g., `"C:\\Users\\Alice\\Desktop\\pwd.txt"`)
  2) `name` (e.g., `"pwd.txt"`)
  3) `keyword` (e.g., `"pwd"`)
- Fields are quoted with double quotes. Internal quotes are escaped by doubling them (CSV standard).

### Examples

- Scan drive D: with Chinese keywords, save to file:
```
finder.exe -k 密码,账户 -o result.csv D:\
```

- Scan two folders with 8 threads, recursive, do not follow symlinks, print hits to console:
```
finder.exe -t 8 --debug C:\\Users\\Alice\\Downloads C:\\Data
```

- Use default keywords and scan all drives:
```
finder.exe -all
```

### Behavior and Performance

- Work is scheduled via a multithreaded directory queue; subdirectories are enqueued as discovered.
- Thread-safe output: a global mutex ensures each record is written atomically.
- The output file is opened once at the first write (or at startup) and truncated; use `-o` for a custom file.

### Access Denied / WOW64 Notes

- Directories without access permissions are skipped; with `--debug-denied`, paths are logged as `DENIED: <path>`.
- 32-bit builds on 64-bit Windows may hit WOW64 redirection for `C:\Windows\System32`. Use 64-bit build or `C:\Windows\Sysnative` if you must read the native 64-bit system directory.

### Exit Codes

- `0`: Completed successfully.
- Non-zero: Abnormal termination or unhandled runtime error (rare).

### Project Structure

```
.
├─ include/
│  ├─ app.h              # run_app entry
│  ├─ config.h           # Config & parse_args
│  ├─ output.h           # output & counters
│  ├─ scanner.h          # workers & queue
│  └─ utils.h            # utilities (paths, encoding, timestamp)
├─ src/
│  ├─ app.cpp            # main flow (parse, start, summary)
│  ├─ config.cpp         # argument parsing
│  ├─ output.cpp         # CSV output & debug printing
│  ├─ scanner.cpp        # multithread scan & scheduling
│  └─ utils.cpp          # helpers implementation
├─ main.cpp              # minimal entry that calls run_app
├─ build.ps1             # one-click build (privacy, clean)
├─ build.cmd             # cmd wrapper for build.ps1
└─ README.md             # Chinese guide
```

### Contributing

- Ensure `./build.ps1` builds successfully and only outputs `finder.exe`.
- Keep reproducible builds (`/Brepro`) and avoid generating PDBs. Do not embed personal paths or timestamps via macros.
- Code style:
  - Use meaningful names; prefer early returns; avoid deep nesting.
  - Windows-specific code lives in `src/`; cross-cutting helpers in `utils`.
- Document changes in `README.md` if behavior or options change. Note any backward compatibility impacts to CSV.


