# Custom Compiler Implementation

This is a personal/academic project to design and implement a custom compiler using **Flex** (Lexical Analyzer) and **Bison** (Syntax Analyzer), ultimately targeting Assembly. It follows the standard phases of compiler design.

---

## 🚀 Current Status: Complete (All 7 Phases Finished)

The project has advanced all the way through **Phase 1 (Lexical Analysis)**, **Phase 2 (Syntax Analysis)**, **Phase 3 (Abstract Syntax Tree)**, **Phase 4 (Semantic Analysis)**, **Phase 5 (TAC Generation)**, **Phase 6 (Code Optimization)**, and has successfully conquered the final tier: **Phase 7 (Target Code Generation)** mapping out native RISC-V machine assembly. 

The Lexer streams tokens, the Parser validates the grammar and constructs a structural AST, the Semantic Analyzer validates token types and scope logic, the Custom TAC Generator resolves that AST into linearly executable intermediate Three-Address Code, the **Optimizer** aggressively pre-computes constant logic before runtime, and finally, the **Assembly Generator** builds `ecall`-ready RISC-V (RV32IM) machine logic.

### Supported Features
The grammar has been expanded to support more complex C-like structures and perfectly match intermediate logic instructions:
*   **Functions:** Definition of functions (e.g., `int main() { ... }`).
*   **Keywords:** `int`, `float`, `if`, `else`, `while`, `for`, `break`, `continue`, `return`, `print`
*   **Data Types:** Identifiers (variable names), Integer Numbers, Floating-Point Numbers, and Strings.
*   **Semantic Checking:** Strict type checking and scope tracing preventing mismatched assignments and undeclared usage.
*   **Arithmetic & Relational Operators:** `+`, `-`, `*`, `/`, `=`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `&&`, `||`, `!`, `++`, `--`
*   **Constant Folding Optimization:** Pre-calculates static nested AST binary/unary operations directly into primitive results, stripping bloated temporary variables from the TAC backend.
*   **Native Output Functions:** Custom parsing and context-switching for the `print` wrapper mapped straight into Venus OS environment calls (`ecall`).
*   **Control Flow:** `if`, `if-else`, `while`, and `for` loop blocks dynamically resolving into `goto` labels natively.
*   **Target Machine:** Generates clean, comment-annotated **RISC-V 32-bit (RV32IM)** Assembly with full stack-pointer, framing, and memory block architecture natively executable on Venus / RARS / Mars emulators.
*   **Punctuation/Symbols:** `(`, `)`, `{`, `}`, `;`

### Project Structure
| File | Description |
| ---- | ----------- |
| `lexer.l` | The Flex source code. Defines regex patterns and loudly dumps Phase 1 tokens (e.g., `INT`, `ID`). |
| `parser.y` | The Bison source code. Contains the Token definitions, Operator Precedence, Grammar Rules tracking the root AST node, and the main phased loop. |
| `ast.h` / `ast.c` | Custom strictly binary tree mapping to allocate, link, and beautifully print the ASCII structural tree branches. |
| `symtab.h` / `symtab.c` | The Semantic Analyzer managing scopes, checking types (int vs string), and halting on errors. |
| `tac.h` / `tac.c` | The TAC generator resolving branches into linearly executable logical components using temporary variables. |
| `opt.h` / `opt.c` | The Code Optimizer, navigating bottom-up through the AST natively collapsing dead logic and folding constants. |
| `asm.h` / `asm.c` | Phase 7 Assembly Backend directly targeting RISC-V 32-bit CPU architectures. |
| `test.txt` | A sample piece of source code used to test the full 7-phase integration pipeline. |

---

## 💻 How to Build and Run (Windows)

We use **WinFlexBison** (or equivalent `flex` and `bison` ports) along with `gcc` (MinGW) on Windows.

**1. Generate the Parser and Lexer Code**
```powershell
win_bison -d parser.y
win_flex lexer.l
```

**2. Compile into an executable**
*(Notice we are now compiling `opt.c` and `asm.c` as well)*
```powershell
gcc parser.tab.c lex.yy.c ast.c tac.c symtab.c opt.c asm.c -o compiler.exe
```

**3. Run the Compiler against the sample test code**
```powershell
.\compiler.exe test.txt
```

### Example Test File (`test.txt`)
```c
int main() {
    int x = 10 + 5 * 2;
    int y = x + (100 / 10);
    print y;
    print ("hello");
    return 0;
}
```

### Expected Output
The compiler now dumps all 6 phases sequentially in one run, demonstrating the raw pre-optimized vs post-optimized configurations!
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
            └── Decl(x)
                ├── Type(int)
                └── Assign
                    ├── ID(x)
                    ├── BinOp(+)
                    │   ├── Value(10)
                    │   └── BinOp(*)
                    │       ├── Value(5)
                    │       └── Value(2)
                    └── Decl(y)
                        ├── Type(int)
                        └── Assign
                            ├── ID(y)
                            ├── BinOp(+)
                            │   ├── ID(x)
                            │   └── BinOp(/)
                            │       ├── Value(100)
                            │       └── Value(10)
                            └── Return
                                └── ID(y)

=========================================
   PHASE 4: SEMANTIC ANALYSIS (SYMBOL)
=========================================
NAME                 KIND            SCOPE      STATUS
----------------------------------------------------------
main                 Function        0          Active
x                    int             1          Closed
y                    int             1          Closed

Semantics validated successfully. No errors found.

=========================================
   PHASE 5: INTERMEDIATE CODE (TAC)
=========================================

main:
BeginFunc
t0 = 5 * 2
t1 = 10 + t0
x = t1
t2 = 100 / 10
t3 = x + t2
y = t3
Return y
EndFunc

=========================================
   PHASE 6: CODE OPTIMIZATION (AST/TAC)
=========================================
Applying Constant Folding Optimization...

[ Optimized AST Structure ]
Program
└── Function(main)
    └── Decl(x)
        ├── Type(int)
        └── Assign
            ├── ID(x)
            ├── Value(20)
            └── Decl(y)
                ├── Type(int)
                └── Assign
                    ├── ID(y)
                    ├── BinOp(+)
                    │   ├── ID(x)
                    │   └── Value(10)
                    └── Return
                        └── ID(y)

[ Optimized Intermediate Code (TAC) ]

main:
BeginFunc
x = 20
t0 = x + 10
y = t0
Return y
EndFunc

=========================================
   PHASE 7: TARGET CODE GENERATION (ASM)
=========================================
; === RISC-V 32-bit Architecture ===
.text
.globl main

main:
    addi sp, sp, -256
    sw s0, 252(sp)
    addi s0, sp, 256
    ; ... [snipped framing setups for brevity]
    li a0, 1
    ecall
    li a1, 10
    li a0, 11
    ecall
    .data
.L0_str:
    .asciiz "hello"
    .text
    la a1, .L0_str
    li a0, 4
    ecall
    li a1, 10
    li a0, 11
    ecall
    li a0, 0
    addi sp, sp, -4
    sw a0, 0(sp)
    lw a1, 0(sp)
    addi sp, sp, 4
    li a0, 10
    ecall

-----------------------------------------
Compilation pipeline executed successfully.
```

---

## 🔜 Next Steps
1. The 7-Phase implementation is formally complete.
2. Next logical directions involve extending the architecture registry map or porting this into LLVM targeting architectures like arm64.
