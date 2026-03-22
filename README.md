# LUMA Compiler

This is a comprehensive project to design and implement a custom compiler using **Flex** (Lexical Analyzer) and **Bison** (Syntax Analyzer), ultimately targeting RISC-V Assembly. It follows the standard phases of compiler design and integrates unique, advanced features.

---

## 🚀 Status: Complete (All 7 Phases Finished)

The project cleanly executes through all compilation stages:
**Phase 1 (Lexical Analysis)** → **Phase 2 (Syntax Analysis)** → **Phase 3 (Abstract Syntax Tree)** → **Phase 4 (Semantic Analysis)** → **Phase 5 (TAC Generation)** → **Phase 6 (Code Optimization)** → **Phase 7 (Target Code Generation)**. 

### 🌟 Core & Advanced Features

The grammar supports standard C-like architectures, augmented with modern conveniences:

* **Core Language Attributes:** `num`, `flag` (bool), `check` (if), `otherwise` (else), `while`, `for`, `break`, `continue`, `return`, `show` (print).
* **Time-Travel Debugging:** Native `watch` and `rewind` operators track state mutating through a history stack natively on the assembly environment, enabling reverse-execution of variable states without external debuggers.
* **Optimized Register Allocation:** Hardware expressions are evaluated in RISC-V `t0-t6` registers, minimizing unnecessary stack interactions.
* **Semantic Security:** Undeclared usage rejection and strict type verification dynamically tracked per scope level.
* **Constant Folding:** Mathematical constants (`5 * 10`) evaluated locally inside the AST before assembly emission.

---

### Project Structure

| File | Description |
| ---- | ----------- |
| `lexer.l` | Flex source code. Maps raw strings to recognized LUMA tokens. |
| `parser.y` | Bison source. Defines grammar rules and hierarchical language structure. |
| `ast.h` / `ast.c` | Abstract Syntax Tree engine featuring an ASCII tree printer. |
| `symtab.h` / `symtab.c` | Semantic Analyzer ensuring accurate variable scoping and watch/rewind tracking. |
| `tac.h` / `tac.c` | Three-Address-Code (TAC) generation resolving branches into intermediate math. |
| `opt.h` / `opt.c` | Code Optimizer for resolving math constraints. |
| `asm.h` / `asm.c` | Phase 7 backend mapping TAC logic into pure RISC-V 32-bit Assembly. |
| `test_*.txt` | Multiple sample LUMA source scripts exercising math, loops, logic, and time travel. |
| `Compiler_Report.tex` | A highly polished 10-page LaTeX report documenting the compiler. |

---

## 💻 How to Build and Run (Windows)

The compiler leverages **WinFlexBison** and `gcc` (MinGW).

**1. Generate the Parser and Lexer Code**
```powershell
win_bison -d parser.y
win_flex lexer.l
```

**2. Compile into an Executable**
```powershell
gcc parser.tab.c lex.yy.c ast.c tac.c symtab.c opt.c asm.c -o compiler.exe
```

**3. Run the Compiler**
```powershell
.\compiler.exe test_loop_for.txt
```

### Example LUMA Code (`test_loop_for.txt`)
```c
num start() {
    num sum = 0;
    
    // Nested for-loops mapping a right triangle design
    for (num i = 1; i <= 10; i++) {
        for(num j=i+1;j<=10;j++){
            show("*");
        }
        show("\n");
    }

    return 0;
}
```

### Expected Output
The LUMA compiler automatically traverses all 7 phases, cascading structurally until arriving at the executed Assembly format. Below is the final generated Assembly output (Phase 7), perfectly formatted for the RISC-V simulator Venus.

```assembly
=========================================
   PHASE 7: TARGET CODE GENERATION (ASM)
=========================================
# === RISC-V 32-bit Assembly ===
.data
.L0_str:
    .asciiz "*"
    .word 0
.L1_str:
    .asciiz "\n"
    .word 0
.text
.globl main
    j main

main:
    addi sp, sp, -256
    sw ra, 252(sp)
    sw s0, 248(sp)
    addi s0, sp, 256
    li t0, 0
    sw t0, -12(s0)
    li t0, 1
    sw t0, -16(s0)
.L2:
    lw t0, -16(s0)
    li t1, 10
    slt t0, t1, t0
    xori t0, t0, 1
    beqz t0, .L3
    lw t0, -16(s0)
    li t1, 1
    add t0, t0, t1
    sw t0, -20(s0)
.L4:
    lw t0, -20(s0)
    li t1, 10
    slt t0, t1, t0
    xori t0, t0, 1
    beqz t0, .L5
    la a0, .L0_str
    li a7, 4
    ecall
    lw t0, -20(s0)
    addi t1, t0, 1
    sw t1, -20(s0)
    j .L4
.L5:
    la a0, .L1_str
    li a7, 4
    ecall
    lw t0, -16(s0)
    addi t1, t0, 1
    sw t1, -16(s0)
    j .L2
.L3:
    li t0, 0
    mv a0, t0
    li a7, 10
    ecall
    li a7, 10
    ecall

-----------------------------------------
Compilation pipeline executed successfully.
```
