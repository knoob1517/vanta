# Vanta Programming Language

**Vanta** is a minimalist interpreted programming language written in C. It focuses on stability, UTF-8 support, and clear error reporting.

[![Release](https://img.shields.io/badge/release-golden_version-gold)](https://github.com/knoob1517/vanta/releases/tag/golden_version)

---

## Features

* **One-pass parser**
  Processes code in a single pass with predictable behavior.

* **UTF-8 support**
  Allows non-ASCII variable names:

  ```vanta
  let kết_quả = 10
  ```

* **Smart input**
  `input()` automatically detects number or string.

* **Unary minus support**
  Handles negative values:

  ```vanta
  let x = -144
  ```

* **Error reporting**
  Displays line/column and highlights exact error location.

* **Built-in functions**
  Includes utilities for math, strings, and system control.

---

## Getting Started

### Compile

Requires a C compiler (GCC or MSVC):

```bash
gcc main.c -o vanta.exe
```

### Run

```bash
./vanta.exe [filename]
```

If no file is provided, it defaults to `main.vt`.

---

## Syntax Overview

### Variables and arithmetic

```vanta
let a = 10
let b = 20
let result = (a + b) * 5 / -2
printf(result)
```

### Strings and UTF-8

```vanta
let name = input("What is your name? ")
printf("Welcome, " + name)
printf(upper(name))
```

### Random numbers

```vanta
let n = rand(1, 100)
printf(n)
```

### Power and modulo

```vanta
let x = 1 % 4
printf(x)

let y = 2 ^ 2 ^ 3
printf(y)
```

### If statement

```vanta
let x = 1

if x == 1 {
    printf("x = 1")
}
elseif x == 2 {
    printf("x = 2")
}
else {
    printf("other")
}
```

### While loop

```vanta
let i = 0

while i < 5 {
    printf(i)
    let i = i + 1
}
```

---

## Built-in Functions

| Function         | Description         | Example              |
| ---------------- | ------------------- | -------------------- |
| `printf(val)`    | Print value         | `printf("Hello")`    |
| `input(msg)`     | Read input          | `let x = input("?")` |
| `rand(min, max)` | Random number       | `rand(1, 100)`       |
| `sqrt(n)`        | Square root         | `sqrt(144)`          |
| `abs(n)`         | Absolute value      | `abs(-5)`            |
| `len(str)`       | String length       | `len("Vanta")`       |
| `upper(str)`     | Uppercase           | `upper("vanta")`     |
| `lower(str)`     | Lowercase           | `lower("VANTA")`     |
| `type(val)`      | Type (0=num, 1=str) | `type(10)`           |
| `sleep(ms)`      | Pause execution     | `sleep(1000)`        |
| `clear`          | Clear console       | `clear`              |
| `exit`           | Exit program        | `exit`               |

---

## Project Structure

* `main.c` — interpreter core (lexer, parser, AST, evaluator)
* `test.vt` — test script (optional)

---

## Status

Current release: **Golden Version**

The language is stable for core features, but may still contain edge-case bugs as development continues.

---

## Author

Developed by **k_noob**.
