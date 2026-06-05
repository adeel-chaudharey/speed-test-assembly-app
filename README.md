# Typing Speed Test — Semester Project
## x86-64 Assembly + Qt C++ | Windows 11

---

## Project Overview

| Layer | Technology | Purpose |
|-------|-----------|---------|
| GUI | Qt 5/6 Widgets (C++) | Window, buttons, timer, display |
| Core Logic | x86-64 NASM Assembly | Character analysis, word counting |
| Communication | Temp files (Option A) | Qt writes input; ASM writes output |

---

## Directory Structure

```
TypingSpeedTest/
├── asm/
│   └── typing_analyzer.asm      ← Assembly source (NASM)
├── main.cpp                     ← Qt entry point
├── mainwindow.h                 ← Main window declaration
├── mainwindow.cpp               ← Main window implementation
├── TypingSpeedTest.pro          ← Qt project file (qmake)
├── CMakeLists.txt               ← CMake build file (alternative)
├── build_asm.bat                ← Script to assemble the .asm file
└── README.md                    ← This file
```

---

## Prerequisites

### 1. NASM (Netwide Assembler)
- Download: https://www.nasm.us/pub/nasm/releasebuilds/
- Install the Windows x64 installer
- Add to PATH: `C:\Program Files\NASM\`

Verify:
```
nasm -v
```

### 2. Qt + MinGW
- Download Qt Online Installer: https://www.qt.io/download-open-source
- Install Qt 5.15 or Qt 6.x with **MinGW 64-bit** component
- Qt Creator is included

### 3. MinGW GCC (for linking ASM)
- Usually installed with Qt. Verify:
```
gcc --version
```

---

## Build Instructions

### Step 1 — Assemble the NASM file

Open **Command Prompt** (not PowerShell) in the project root:

```bat
build_asm.bat
```

This produces: `asm\typing_analyzer.exe`

**Manual alternative:**
```bat
nasm -f win64 asm\typing_analyzer.asm -o asm\typing_analyzer.obj
gcc -o asm\typing_analyzer.exe asm\typing_analyzer.obj -lkernel32 -nostartfiles -e main
```

### Step 2 — Build the Qt Application

**Option A — Qt Creator (recommended):**
1. Open Qt Creator
2. File → Open File or Project → select `TypingSpeedTest.pro`
3. Configure kit: Desktop Qt (MinGW 64-bit)
4. Build → Build Project (Ctrl+B)
5. Run (Ctrl+R)

**Option B — Command line (qmake):**
```bat
qmake TypingSpeedTest.pro
mingw32-make
```

**Option C — CMake:**
```bat
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
mingw32-make
```

### Step 3 — Ensure exe placement

After building, `typing_analyzer.exe` must be in the **same folder** as `TypingSpeedTest.exe`.

The `.pro` file and `CMakeLists.txt` both include a post-build copy step that handles this automatically.

---

## How It Works

### Typing Test Flow

```
User clicks Start
        │
        ▼
Qt picks random sentence → displays it
Qt starts QElapsedTimer + QTimer (display refresh)
        │
User types in the text box
        │
User clicks Submit
        │
        ▼
Qt stops timer, records elapsed seconds
Qt writes two-line input file to %TEMP%\typing_in.txt:
    Line 1: original sentence
    Line 2: user-typed text
        │
        ▼
Qt launches typing_analyzer.exe via QProcess:
    typing_analyzer.exe "C:\...\typing_in.txt" "C:\...\typing_out.txt"
        │
Assembly reads file, analyzes, writes result file:
    Line 1: correct_chars
    Line 2: incorrect_chars
    Line 3: total_chars
    Line 4: word_count
        │
        ▼
Qt reads result file, calculates:
    Accuracy % = (correct × 100) / total
    WPM        = word_count / (elapsed_seconds / 60)
        │
        ▼
Qt displays all results in the Results panel
```

### Assembly Functions

| Function | Description |
|----------|-------------|
| `parse_args` | Extracts input/output file paths from command line |
| `copy_line` | Copies one line from file buffer, handles CR/LF |
| `analyze_text` | Main loop: character comparison + word counting |
| `build_output` | Formats 4 integers as text lines |
| `append_decimal` | Converts 64-bit integer to ASCII decimal (no CRT) |

---

## Example Run

**Sentence:** `Assembly language is powerful`
**Typed:**    `Assembly language is powerfull`

**Assembly outputs:**
```
28
1
29
4
```

**Qt calculates:**
```
Accuracy : (28 × 100) / 29 = 96.6%
WPM      : 4 / (15/60)     = 16
```

**Displayed:**
```
Correct Characters:   28
Incorrect Characters:  1
Accuracy:           96.6%
WPM:                  16
Time Taken:         15.0 sec
```

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| `nasm: command not found` | Add NASM to PATH or use full path |
| `gcc: command not found` | Add MinGW to PATH |
| Linking error | Try `ld` instead of `gcc`; see build_asm.bat |
| `typing_analyzer.exe not found` | Copy it next to TypingSpeedTest.exe manually |
| Qt Creator can't find kit | Configure a Desktop Qt MinGW 64-bit kit in Qt Creator settings |
| App opens then crashes | Run from Qt Creator in Debug mode; check QProcess exit code |

---

## Grading Checklist

- [x] GUI implemented in Qt Widgets (C++)
- [x] Core logic in x86-64 NASM Assembly
- [x] Assembly performs: character comparison, correct count, incorrect count, total count, word count
- [x] Qt handles: timer, accuracy calculation, WPM calculation, result display
- [x] Communication via temp files (Option A — recommended)
- [x] Runs on Windows 11
- [x] No Python / WebAssembly / WSL / networking dependencies
