# Custom Compiler Implementation

This is a personal/academic project to design and implement a custom compiler using **Flex** (Lexical Analyzer) and **Bison** (Syntax Analyzer), ultimately targeting Assembly. It follows the standard phases of compiler design.

---

## 🚀 Current Status: Phase 2 (Syntax Analysis / Parsing)

The project has advanced from Lexical Analysis into **Phase 2: Syntax Analysis**. The **Parser** (built with Bison) now directs the **Lexer** (built with Flex). The Lexer streams tokens to the Parser, which verifies that the token sequence forms valid grammatical statements according to our defined language rules.

### Supported Features (Phase 2)
The grammar has been expanded to support more complex C-like structures:
*   **Functions:** Definition of functions (e.g., `int main() { ... }`).
*   **Keywords:** `int`, `float`, `if`, `else`, `return`
*   **Data Types:** Identifiers (variable names), Integer Numbers, and Floating-Point Numbers.
*   **Arithmetic & Relational Operators:** `+`, `-`, `*`, `/`, `=`, `==`
*   **Control Flow:** `if` and `if-else` blocks.
*   **Punctuation/Symbols:** `(`, `)`, `{`, `}`, `;`

### Project Structure (Phase 2)
| File | Description |
| ---- | ----------- |
| `lexer.l` | The Flex source code. Defines regex patterns and returns token types (e.g., `T_INT`, `T_ID`) defined by the parser. |
| `parser.y` | The Bison source code. Contains the Token definitions, Operator Precedence, Grammar Rules, and the main execution loop. |
| `test.txt` | A sample piece of source code used to test the Lexer and Parser integration. |
| `.gitignore` | Ignores auto-generated files like `lex.yy.c`, `parser.tab.*`, and executables to keep the repository clean. |

---

## 💻 How to Build and Run (Windows)

We use **WinFlexBison** (or equivalent `flex` and `bison` ports) along with `gcc` (MinGW) on Windows.

**1. Generate the Parser C code and Header file**
```powershell
win_bison -d parser.y
```
*(This generates `parser.tab.c` and `parser.tab.h`)*

**2. Generate the Lexer C code**
```powershell
win_flex lexer.l
```
*(This generates `lex.yy.c`)*

**3. Compile both together into an executable**
```powershell
gcc parser.tab.c lex.yy.c -o compiler.exe
```

**4. Run the Compiler against the sample test code**
```powershell
.\compiler.exe test.txt
```

### Example Test File (`test.txt`)
```c
int main() {
    int x = 5;
    float y = 3.14;

    if (x == 5) {
        y = y + 1;
    }

    return 0;
}
```

### Expected Output
If the grammar rules are successfully matched, the output will trace the parsing steps:
```text
Starting Syntax Analysis (Parsing)...
...
Parsed: Variable Declaration with Initialization
...
Parsed: If Block
...
Parsed: Return Statement
Parsed: Function Definition
Program successfully parsed!
Finished Parsing.
```

---

## 🔜 Next Steps
1. Advance to **Phase 3: Semantic Analysis / AST Generation** to build an Abstract Syntax Tree.
2. Introduce a symbol table to track variable types and assignments.
3. Handle type checking (e.g., ensuring you can't add an `int` to a struct without conversion).
