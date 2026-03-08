# Custom Compiler Implementation

This is a personal/academic project to design and implement a custom compiler using **Flex** (Lexical Analyzer) and **Bison** (Syntax Analyzer), ultimately targeting Assembly. It follows the standard phases of compiler design.

---

## 🚀 Current Status: Phase 5 (Intermediate Code Generation - TAC)

The project has advanced through **Phase 1 (Lexical Analysis)**, **Phase 2 (Syntax Analysis)**, **Phase 3 (Abstract Syntax Tree)**, **Phase 4 (Semantic Analysis)** and is now actively generating **Phase 5: Intermediate Code Generation (Three-Address Code)**. 

The Lexer streams tokens, the Parser validates the grammar and constructs a strict binary AST representation, the Semantic Analyzer validates token types and scope logic, and the custom TAC generator linearly resolves that AST into intermediate Three-Address Code instructions format.

### Supported Features
The grammar has been expanded to support more complex C-like structures and perfectly match intermediate logic instructions:
*   **Functions:** Definition of functions (e.g., `int main() { ... }`).
*   **Keywords:** `int`, `float`, `if`, `else`, `while`, `for`, `break`, `continue`, `return`
*   **Data Types:** Identifiers (variable names), Integer Numbers, Floating-Point Numbers, and Strings.
*   **Semantic Checking:** Strict type checking and scope tracing preventing mismatched assignments and undeclared usage.
*   **Arithmetic & Relational Operators:** `+`, `-`, `*`, `/`, `=`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `&&`, `||`, `!`, `++`, `--`
*   **Control Flow:** `if`, `if-else`, `while`, and `for` loop blocks dynamically resolving into `goto` labels natively.
*   **Punctuation/Symbols:** `(`, `)`, `{`, `}`, `;`

### Project Structure
| File | Description |
| ---- | ----------- |
| `lexer.l` | The Flex source code. Defines regex patterns and loudly dumps Phase 1 tokens (e.g., `INT`, `ID`). |
| `parser.y` | The Bison source code. Contains the Token definitions, Operator Precedence, Grammar Rules tracking the root AST node, and the main phased loop. |
| `ast.h` / `ast.c` | Custom strictly binary tree mapping to allocate, link, and beautifully print the ASCII structural tree branches. |
| `symtab.h` / `symtab.c` | The Semantic Analyzer managing scopes, checking types (int vs string), and halting on errors. |
| `tac.h` / `tac.c` | The TAC generator resolving binary branches into linearly executable logical components using temporary variables. |
| `test.txt` | A sample piece of source code used to test the full 5-phase integration. |

---

## 💻 How to Build and Run (Windows)

We use **WinFlexBison** (or equivalent `flex` and `bison` ports) along with `gcc` (MinGW) on Windows.

**1. Generate the Parser and Lexer Code**
```powershell
win_bison -d parser.y
win_flex lexer.l
```

**2. Compile into an executable**
*(Notice we are now compiling `tac.c` and `symtab.c` as well)*
```powershell
gcc parser.tab.c lex.yy.c ast.c tac.c symtab.c -o compiler.exe
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
The compiler now dumps all 5 phases sequentially in one run:
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
└── Block
    └── Function(main)
        └── Block
            ├── Decl(x)
            │   └── Type(int)
            └── Block
                ├── Assign
                │   ├── ID(x)
                │   └── Value(5)
                └── Block
                    └── Return
                        └── ID(x)

=========================================
   PHASE 4: SEMANTIC ANALYSIS (SYMBOL)
=========================================
NAME                 KIND            SCOPE      STATUS
----------------------------------------------------------
main                 Function        0          Active
x                    int             1          Closed

Semantics validated successfully. No errors found.

=========================================
   PHASE 5: INTERMEDIATE CODE (TAC)
=========================================

main:
BeginFunc
x = 5
Return x
EndFunc

-----------------------------------------
Compilation pipeline executed successfully.
```

---

## 🔜 Next Steps
1. Advance to **Phase 6: Code Optimization** (Constant folding, dead code elimination).
2. Advance to **Phase 7: Target Code Generation (Assembly)** translating TAC logic into real machine instructions.
