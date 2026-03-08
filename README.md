# Custom Compiler Implementation

This is a personal/academic project to design and implement a custom compiler using **Flex** (Lexical Analyzer) and **Bison** (Syntax Analyzer), ultimately targeting Assembly. It follows the standard phases of compiler design.

---

## 🚀 Current Status: Phase 3 (Abstract Syntax Tree Generation)

The project has advanced through **Phase 1 (Lexical Analysis)** and **Phase 2 (Syntax Analysis)** and is now fully generating a visual **Phase 3: Abstract Syntax Tree (AST)**. The Lexer streams tokens, the Parser validates the grammar, and the parser rules construct a memory-mapped AST which is printed using beautiful ASCII tree characters.

### Supported Features
The grammar has been expanded to support more complex C-like structures:
*   **Functions:** Definition of functions (e.g., `int main() { ... }`).
*   **Keywords:** `int`, `float`, `if`, `else`, `while`, `for`, `break`, `continue`, `return`
*   **Data Types:** Identifiers (variable names), Integer Numbers, and Floating-Point Numbers.
*   **Arithmetic & Relational Operators:** `+`, `-`, `*`, `/`, `=`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `&&`, `||`, `!`, `++`, `--`
*   **Control Flow:** `if`, `if-else`, `while`, and `for` loop blocks.
*   **Punctuation/Symbols:** `(`, `)`, `{`, `}`, `;`

### Project Structure
| File | Description |
| ---- | ----------- |
| `lexer.l` | The Flex source code. Defines regex patterns and loudly dumps Phase 1 tokens (e.g., `INT`, `ID`). |
| `parser.y` | The Bison source code. Contains the Token definitions, Operator Precedence, Grammar Rules tracking the root AST node, and the main phased loop. |
| `ast.h` / `ast.c` | Custom code to allocate, map, and beautifully print the ASCII structural tree branches. |
| `test.txt` | A sample piece of source code used to test the full Lexer -> Parser -> AST integration. |

---

## 💻 How to Build and Run (Windows)

We use **WinFlexBison** (or equivalent `flex` and `bison` ports) along with `gcc` (MinGW) on Windows.

**1. Generate the Parser and Lexer Code**
```powershell
win_bison -d parser.y
win_flex lexer.l
```

**2. Compile into an executable**
```powershell
gcc parser.tab.c lex.yy.c ast.c -o compiler.exe
```

**3. Run the Compiler against the sample test code**
```powershell
.\compiler.exe test.txt
```

### Example Test File (`test.txt`)
```c
int main() {
    int x = 5;
    return x;
}
```

### Expected Output
The compiler now dumps all 3 phases sequentially in one run:
```text
=========================================
   PHASE 1: LEXICAL ANALYSIS (TOKENS)
=========================================
INT        int
ID         main
LPAREN     (
RPAREN     )
...

=========================================
   PHASE 2: SYNTAX ANALYSIS (PARSING)
=========================================
Grammar validated successfully. No syntax errors found.

=========================================
   PHASE 3: ABSTRACT SYNTAX TREE (AST)
=========================================
Program
└── Function(main)
    └── Body
        ├── Decl(x)
        │   └── Value(5)
        └── Return
            └── ID(x)

-----------------------------------------
Compilation pipeline executed successfully.
```

---

## 🔜 Next Steps
1. Advance to **Semantic Analysis / Context validation**.
2. Introduce a symbol table to track variable types and assignments.
3. Handle type checking (e.g., preventing type mismatches silently).
