# Custom Compiler Implementation

This is a personal/academic project to design and implement a custom compiler using **Flex** (Lexical Analyzer) and **Bison** (Syntax Analyzer), ultimately targeting Assembly. It follows the standard phases of compiler design.

---

## 🚀 Current Status: Complete (All 7 Phases Finished)

The project has advanced all the way through **Phase 1 (Lexical Analysis)**, **Phase 2 (Syntax Analysis)**, **Phase 3 (Abstract Syntax Tree)**, **Phase 4 (Semantic Analysis)**, **Phase 5 (TAC Generation)**, **Phase 6 (Code Optimization)**, and has successfully conquered the final tier: **Phase 7 (Target Code Generation)** mapping out native RISC-V machine assembly. 

The Lexer streams tokens, the Parser validates the grammar and constructs a structural AST, the Semantic Analyzer validates token types and scope logic, the Custom TAC Generator resolves that AST into linearly executable intermediate Three-Address Code, the **Optimizer** aggressively pre-computes constant logic before runtime, and finally, the **Assembly Generator** builds explicit, highly-efficient RISC-V (RV32IM) machine logic supporting multiple functions, stack frames, and register allocation.

### Supported Features
The grammar has been heavily expanded to support complex C-like architectures:
*   **Functions & Parameters:** Definition of multiple functions, argument passing across registers, properly isolated stack pointer memory offsets, and jump-return address state saving.
*   **Keywords:** `int`, `float`, `if`, `else`, `while`, `for`, `break`, `continue`, `return`, `print`
*   **Data Types:** Identifiers (variable names), Integer Numbers, Floating-Point Numbers, and Strings.
*   **Semantic Checking:** Strict type checking, argument validation, and bounded scope tracing.
*   **Arithmetic & Relational Operators:** `+`, `-`, `*`, `/`, `=`, `==`, `!=`, `<`, `>`, `<=`, `>=`, `&&`, `||`, `!`, `++`, `--`
*   **Constant Folding Optimization:** Pre-calculates static nested AST binary/unary operations directly into primitive results, stripping bloated temporary variables from the TAC backend.
*   **Native Output Functions:** Smart separation of strings into `.data` blocks and mapped cleanly into Venus OS environment calls (`ecall`).
*   **Control Flow:** Native hardware branching for `if-else`, `while`, and `for` loop blocks dynamically resolving into assembler `.L` labels.
*   **Physical Register Allocator:** Evaluates expressions directly in hardware CPU registers (`t0`-`t6`) rather than doing slow stack loads and saves for every arithmetic step.

### Project Structure
| File | Description |
| ---- | ----------- |
| `lexer.l` | The Flex source code. Defines regex patterns and loudly dumps Phase 1 tokens (e.g., `INT`, `ID`). |
| `parser.y` | The Bison source code. Contains the Token definitions, Operator Precedence, and Grammar Rules. |
| `ast.h` / `ast.c` | Custom strictly binary tree mapping to allocate, link, and beautifully print the ASCII structural tree branches. |
| `symtab.h` / `symtab.c` | The Semantic Analyzer managing scopes, checking types, adding formal parameters, and halting on errors. |
| `tac.h` / `tac.c` | The TAC generator resolving branches into intermediate variables. |
| `opt.h` / `opt.c` | The Code Optimizer, navigating bottom-up through the AST natively collapsing dead logic and folding constants. |
| `asm.h` / `asm.c` | Phase 7 Assembly Backend targeting RISC-V 32-bit CPUs. Understand multi-function framing and caller/callee save requirements. |
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
```powershell
gcc parser.tab.c lex.yy.c ast.c tac.c symtab.c opt.c asm.c -o compiler.exe
```

**3. Run the Compiler against the sample test code**
```powershell
.\compiler.exe test.txt
```

### Example Test File (`test.txt`)
```c
int mul(int val1, int val2, int offset) {
    int result = (val1 * val2) + offset;
    return result;
}

int main() {
    print("Test: Function Calls & Arguments");
    
    int x = 10;
    int y = 5;
    
    int answer1 = mul(x, y, 7);
    int answer2 = mul(100, 2, 0);
    
    print("10 * 5 + 7 =");
    print(answer1);
    
    print("100 * 2 + 0 =");
    print(answer2);
    
    return 0;
}
```

### Expected Output
The compiler now dumps all 7 phases sequentially in one run, culminating in perfect simulation-ready RISC-V execution format:
```text
=========================================
   PHASE 7: TARGET CODE GENERATION (ASM)
=========================================
; === RISC-V 32-bit Architecture ===
.data
.L0_str:
    .asciiz "Test: Function Calls & Arguments"
    .byte 0
.L1_str:
    .asciiz "10 * 5 + 7 ="
    .byte 0
.L2_str:
    .asciiz "100 * 2 + 0 ="
    .byte 0
.text
.globl main
    j main

mul:
    addi sp, sp, -256
    sw ra, 252(sp)
    sw s0, 248(sp)
    addi s0, sp, 256
    sw a0, -12(s0)
    sw a1, -16(s0)
    sw a2, -20(s0)
    lw t0, -12(s0)
    lw t1, -16(s0)
    mul t0, t0, t1
    lw t1, -20(s0)
    add t0, t0, t1
    sw t0, -24(s0)
    lw t0, -24(s0)
    mv a0, t0
    lw ra, 252(sp)
    lw s0, 248(sp)
    addi sp, sp, 256
    jr ra

main:
    addi sp, sp, -256
    sw ra, 252(sp)
    sw s0, 248(sp)
    addi s0, sp, 256
    la a0, .L0_str
    li a7, 4
    ecall
    ...
```

---

## 🔜 Next Steps
1. The 7-Phase implementation is formally complete.
2. Next logical directions involve extending the architecture registry map or porting this into LLVM targeting architectures like arm64.
