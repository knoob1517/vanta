# 🌌 Vanta Programming Language

**Vanta** is a minimalist interpreted programming language written entirely in C. This project focuses on **stability**, **UTF-8 support** (allowing for localized variable names), and an intuitive, visual error-reporting system.

[![Release](https://img.shields.io/badge/release-golden_version-gold)](https://github.com/knoob1517/vanta/releases/tag/golden_version)

## ✨ Key Features

- **Stable One-Pass Parser**: Optimized to process code in a single pass without deep recursion, preventing infinite loops and ensuring high reliability.
- **Full UTF-8 Support**: Supports non-ASCII characters in variable names (e.g., `let kết_quả = 10` or `let lượng_mưa = 50`).
- **Smart Input**: The `input()` function automatically detects whether the user entered a **Number** or a **String**, eliminating the need for manual casting.
- **Negative Number Support**: Full support for unary minus operations (e.g., `let x = -144`).
- **Visual Error Reporting**: Provides GCC-style error messages with line/column numbers and a `^` pointer pointing directly to the syntax error.
- **Rich Built-in Library**: Integrated functions for string manipulation, mathematics, and system control.

## 🚀 Getting Started

### 1. Compilation
You only need a C compiler (GCC or MSVC) on Windows:
```bash
gcc main.c -o vanta.exe
```

### 2. Running a Script
By default, the interpreter looks for the file in filename and executes it in the same directory:
```bash
./vanta.exe [filename]
```

You can modify filename to execute more script!

## 📝 Basic Syntax

### Variables and Arithmetic
```vanta
# This is a comment
let a = 10
let b = 20
let result = (a + b) * 5 / -2
printf(result)
```

### Strings and UTF-8 Identifiers
```vanta
let tên = input("What is your name? ")
printf("Welcome, " + tên)
printf(upper(tên)) # Convert to uppercase
```

### Random Numbers (Two Parameters)
```vanta
let lucky_num = rand(1, 100)
printf("Your lucky number between 1-100 is:")
printf(lucky_num)
```

## 📚 Built-in Functions

| Function | Description | Example |
| :--- | :--- | :--- |
| `printf(val)` | Prints value to console | `printf("Hello")` |
| `input(msg)` | Gets user input with prompt | `let x = input("?")` |
| `rand(min, max)`| Generates random number in range | `rand(1, 100)` |
| `sqrt(n)` | Calculates square root | `sqrt(144)` |
| `abs(n)` | Returns absolute value | `abs(-5)` |
| `len(str)` | Returns string length | `len("Vanta")` |
| `upper(str)` | Converts string to uppercase | `upper("vanta")` |
| `lower(str)` | Converts string to lowercase | `lower("VANTA")` |
| `type(val)` | Returns type (0: Num, 1: Str) | `type(10)` |
| `sleep(ms)` | Pauses execution (milliseconds) | `sleep(2000)` |
| `clear` | Clears the console screen | `clear` |
| `exit` | Terminates the program | `exit` |

## 🛠 Project Structure
- `main.c`: Core interpreter source code (Lexer, Parser, AST, Evaluator).
- `test.vt`: The standard testing script for all Vanta features. (not available currently)

## 🤝 Contribution
The project is currently in its **Golden Version** (Stable). Contributions regarding control flow (`if-else`), loops (`while`), or arrays are welcome for future releases.

---
**Vanta Language** - Developed by **k_noob** with a passion for programming language theory.
