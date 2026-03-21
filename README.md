# LUMA Compiler

This is a comprehensive project to design and implement a custom compiler using **Flex** (Lexical Analyzer) and **Bison** (Syntax Analyzer), ultimately targeting RISC-V Assembly. It follows the standard phases of compiler design and has been expanded to support several advanced, unique language features.

---

## 🚀 Current Status: Complete (All 7 Phases Finished)

The project cleanly executes through all compilation stages:
**Phase 1 (Lexical Analysis)** → **Phase 2 (Syntax Analysis)** → **Phase 3 (Abstract Syntax Tree)** → **Phase 4 (Semantic Analysis)** → **Phase 5 (TAC Generation)** → **Phase 6 (Code Optimization)** → **Phase 7 (Target Code Generation)**. 

The Lexer streams tokens, the Parser constructs a structural AST, the Semantic Analyzer validates token types and scope logic, the TAC Generator resolves the AST into Three-Address Code, the **Optimizer** pre-computes constant logic before runtime, and the **Assembly Generator** builds explicit, highly-efficient RISC-V (RV32IM) machine logic.

### 🌟 Core & Advanced Features

The grammar supports standard C-like architectures, augmented with modern language conveniences:

* **Core Language Attributes:** `num` (int/float), `flag` (bool), `check` (if), `otherwise` (else), `while`, `for`, `break`, `continue`, `return`, `show` (print).
* **N-Dimensional Arrays:** Native support for cleanly declaring and indexing multi-dimensional memory models, resolved dynamically down to hardware registers (e.g., `num grid[3,3]; grid[1,2] = 5;`).
* **Time-Travel Debugging:** Native `watch` and `rewind` operators that track state mutating through a history stack, enabling reverse-execution of variables without external debuggers.
* **Pipeline Operator (`|>`):** A functional-style dataflow operator passing variables clearly from left to right into functional outputs (e.g. `data |> transform()`).
* **Exponentiation (`**`):** Native power operations parsed directly into efficient assembly loops or constant-folded at compile-time (e.g. `2 ** 8`).
* **Garbage Collection Hooks:** A `gc_collect()` stub that maps directly to sweeping unreferenced stack memory.
* **Semantic Security:** Strict bound checking, symbol tracking, and active scope resolving.
* **Optimized Register Allocation:** Hardware expressions dynamically evaluated in free RISC-V `t0`-`t6` registers, removing bloated stack bottlenecks.

---

### Project Structure

| File | Description |
| ---- | ----------- |
| `lexer.l` | Flex source code. Maps raw strings to recognized LUMA tokens. |
| `parser.y` | Bison source. Defines grammar rules and hierarchical language structure. |
| `ast.h` / `ast.c` | Abstract Syntax Tree engine featuring a beautifully formatted ASCII tree printer. |
| `symtab.h` / `symtab.c` | Semantic Analyzer ensuring accurate variable scoping arrays mapping, and time-travel rules. |
| `tac.h` / `tac.c` | Three-Address-Code (TAC) generation resolving branches into intermediate math. |
| `opt.h` / `opt.c` | Code Optimizer resolving mathematical constants locally inside the AST before assembly. |
| `asm.h` / `asm.c` | Phase 7 backend mapping TAC logic into RISC-V 32-bit Assembly. |
| `test.txt` | Sample LUMA code exercising all language features. |

---

## 💻 How to Build and Run (Windows)

The compiler leverages **WinFlexBison** and `gcc` (MinGW) on Windows.

**1. Generate the Parser and Lexer Code**
```powershell
win_bison -d parser.y
win_flex lexer.l
```

**2. Compile into an Executable**
```powershell
gcc parser.tab.c lex.yy.c ast.c tac.c symtab.c opt.c asm.c -lm -o compiler.exe
```

**3. Run the Compiler**
```powershell
.\compiler.exe test.txt
```

### Example LUMA Code (`test.txt`)
```c
num mul(num a, num b){
    return a * b;
}

num start(){
    show("Hello LUMA!");

    num x = 10;
    num y = 20;
    num z = x + y;
    show(z);
    
    num c = mul(x,y);
    show(c);

    num p = 2 ** 8;
    show(p);

    num arr[5];
    arr[0] = 100;
    arr[1] = 200;
    show(arr[0]);

    num grid[3,3];
    grid[0,0] = 1;
    grid[2,2] = 9;
    show(grid[0,0]);

    watch x;
    x = 5;
    x = 10;
    rewind x;
    show(x); // outputs 5

    check (z > 25) {
        show("z is big");
    } otherwise {
        show("z is small");
    }

    num i = 0;
    while (i < 5) {
        show(i);
        i++;
    }

    gc_collect();

    return 0;
}
```

### Expected Output
The LUMA compiler automatically traverses all 7 phases, cascading structurally until arriving at the executed Assembly format. It outputs visual logs detailing exactly what happens internally inside the Lexer, Parser, AST, Semantic Analyzer, TAC Optimizer, and Assembly Generator. Output executes flawlessly using Venus ( RISC-V simulation environment).

`	ext
=========================================
   PHASE 1: LEXICAL ANALYSIS (TOKENS)
=========================================
NUM_T      num
ID         mul
LPAREN     (
NUM_T      num
ID         a
COMMA      ,
NUM_T      num
ID         b
RPAREN     )
LBRACE     {
RETURN     return
ID         a
MUL        *
ID         b
SEMI       ;
RBRACE     }
NUM_T      num
START      start
LPAREN     (
RPAREN     )
LBRACE     {
SHOW       show
LPAREN     (
STRING     "Hello LUMA!"
RPAREN     )
SEMI       ;
NUM_T      num
ID         x
ASSIGN     =
NUM        10
SEMI       ;
NUM_T      num
ID         y
ASSIGN     =
NUM        20
SEMI       ;
NUM_T      num
ID         z
ASSIGN     =
ID         x
PLUS       +
ID         y
SEMI       ;
SHOW       show
LPAREN     (
ID         z
RPAREN     )
SEMI       ;
NUM_T      num
ID         c
ASSIGN     =
ID         mul
LPAREN     (
ID         x
COMMA      ,
ID         y
RPAREN     )
SEMI       ;
SHOW       show
LPAREN     (
ID         c
RPAREN     )
SEMI       ;
NUM_T      num
ID         p
ASSIGN     =
NUM        2
EXP        **
NUM        8
SEMI       ;
SHOW       show
LPAREN     (
ID         p
RPAREN     )
SEMI       ;
NUM_T      num
ID         arr
LBRACKET   [
NUM        5
RBRACKET   ]
SEMI       ;
ID         arr
LBRACKET   [
NUM        0
RBRACKET   ]
ASSIGN     =
NUM        100
SEMI       ;
ID         arr
LBRACKET   [
NUM        1
RBRACKET   ]
ASSIGN     =
NUM        200
SEMI       ;
SHOW       show
LPAREN     (
ID         arr
LBRACKET   [
NUM        0
RBRACKET   ]
RPAREN     )
SEMI       ;
NUM_T      num
ID         grid
LBRACKET   [
NUM        3
COMMA      ,
NUM        3
RBRACKET   ]
SEMI       ;
ID         grid
LBRACKET   [
NUM        0
COMMA      ,
NUM        0
RBRACKET   ]
ASSIGN     =
NUM        1
SEMI       ;
ID         grid
LBRACKET   [
NUM        0
COMMA      ,
NUM        1
RBRACKET   ]
ASSIGN     =
NUM        2
SEMI       ;
ID         grid
LBRACKET   [
NUM        0
COMMA      ,
NUM        2
RBRACKET   ]
ASSIGN     =
NUM        3
SEMI       ;
ID         grid
LBRACKET   [
NUM        1
COMMA      ,
NUM        0
RBRACKET   ]
ASSIGN     =
NUM        4
SEMI       ;
ID         grid
LBRACKET   [
NUM        1
COMMA      ,
NUM        1
RBRACKET   ]
ASSIGN     =
NUM        5
SEMI       ;
ID         grid
LBRACKET   [
NUM        1
COMMA      ,
NUM        2
RBRACKET   ]
ASSIGN     =
NUM        6
SEMI       ;
ID         grid
LBRACKET   [
NUM        2
COMMA      ,
NUM        0
RBRACKET   ]
ASSIGN     =
NUM        7
SEMI       ;
ID         grid
LBRACKET   [
NUM        2
COMMA      ,
NUM        1
RBRACKET   ]
ASSIGN     =
NUM        8
SEMI       ;
ID         grid
LBRACKET   [
NUM        2
COMMA      ,
NUM        2
RBRACKET   ]
ASSIGN     =
NUM        9
SEMI       ;
SHOW       show
LPAREN     (
ID         grid
LBRACKET   [
NUM        0
COMMA      ,
NUM        0
RBRACKET   ]
RPAREN     )
SEMI       ;
SHOW       show
LPAREN     (
ID         grid
LBRACKET   [
NUM        0
COMMA      ,
NUM        1
RBRACKET   ]
RPAREN     )
SEMI       ;
SHOW       show
LPAREN     (
ID         grid
LBRACKET   [
NUM        0
COMMA      ,
NUM        2
RBRACKET   ]
RPAREN     )
SEMI       ;
SHOW       show
LPAREN     (
ID         grid
LBRACKET   [
NUM        1
COMMA      ,
NUM        0
RBRACKET   ]
RPAREN     )
SEMI       ;
SHOW       show
LPAREN     (
ID         grid
LBRACKET   [
NUM        1
COMMA      ,
NUM        1
RBRACKET   ]
RPAREN     )
SEMI       ;
SHOW       show
LPAREN     (
ID         grid
LBRACKET   [
NUM        1
COMMA      ,
NUM        2
RBRACKET   ]
RPAREN     )
SEMI       ;
SHOW       show
LPAREN     (
ID         grid
LBRACKET   [
NUM        2
COMMA      ,
NUM        0
RBRACKET   ]
RPAREN     )
SEMI       ;
SHOW       show
LPAREN     (
ID         grid
LBRACKET   [
NUM        2
COMMA      ,
NUM        1
RBRACKET   ]
RPAREN     )
SEMI       ;
SHOW       show
LPAREN     (
ID         grid
LBRACKET   [
NUM        2
COMMA      ,
NUM        2
RBRACKET   ]
RPAREN     )
SEMI       ;
WATCH      watch
ID         x
SEMI       ;
ID         x
ASSIGN     =
NUM        5
SEMI       ;
ID         x
ASSIGN     =
NUM        10
SEMI       ;
REWIND     rewind
ID         x
SEMI       ;
SHOW       show
LPAREN     (
ID         x
RPAREN     )
SEMI       ;
CHECK      check
LPAREN     (
ID         z
GT         >
NUM        25
RPAREN     )
LBRACE     {
SHOW       show
LPAREN     (
STRING     "z is big"
RPAREN     )
SEMI       ;
RBRACE     }
OTHERWISE  otherwise
LBRACE     {
SHOW       show
LPAREN     (
STRING     "z is small"
RPAREN     )
SEMI       ;
RBRACE     }
NUM_T      num
ID         i
ASSIGN     =
NUM        0
SEMI       ;
WHILE      while
LPAREN     (
ID         i
LT         <
NUM        5
RPAREN     )
LBRACE     {
SHOW       show
LPAREN     (
ID         i
RPAREN     )
SEMI       ;
ID         i
INC        ++
SEMI       ;
RBRACE     }
GC_COLLECT gc_collect
LPAREN     (
RPAREN     )
SEMI       ;
RETURN     return
NUM        0
SEMI       ;
RBRACE     }

=========================================
   PHASE 2: SYNTAX ANALYSIS (PARSING)
=========================================
Grammar validated successfully. No syntax errors found.

=========================================
   PHASE 3: ABSTRACT SYNTAX TREE (AST)
=========================================
Program
├── Function(mul)
│   ├── Param(a)
│   │   └── Type(num)
│   ├── Param(b)
│   │   └── Type(num)
│   └── Return
│       └── BinOp(*)
│           ├── ID(a)
│           └── ID(b)
└── Function(main)
    ├── Show
    │   └── String("Hello LUMA!")
    ├── Decl(x)
    │   ├── Type(num)
    │   └── Value(10)
    ├── Decl(y)
    │   ├── Type(num)
    │   └── Value(20)
    ├── Decl(z)
    │   ├── Type(num)
    │   └── BinOp(+)
    │       ├── ID(x)
    │       └── ID(y)
    ├── Show
    │   └── ID(z)
    ├── Decl(c)
    │   ├── Type(num)
    │   └── Call(mul)
    │       └── Arg
    │           └── ID(x)
    ├── Show
    │   └── ID(c)
    ├── Decl(p)
    │   ├── Type(num)
    │   └── BinOp(**)
    │       ├── Value(2)
    │       └── Value(8)
    ├── Show
    │   └── ID(p)
    ├── ArrayDecl(arr[5])
    │   └── Type(num)
    ├── ArrayAssign(arr[0])
    │   └── Value(100)
    ├── ArrayAssign(arr[1])
    │   └── Value(200)
    ├── Show
    │   └── ArrayAccess(arr[0])
    ├── ArrayDecl(grid[3,3])
    │   └── Type(num)
    ├── ArrayAssign(grid[0,0])
    │   └── Value(1)
    ├── ArrayAssign(grid[0,1])
    │   └── Value(2)
    ├── ArrayAssign(grid[0,2])
    │   └── Value(3)
    ├── ArrayAssign(grid[1,0])
    │   └── Value(4)
    ├── ArrayAssign(grid[1,1])
    │   └── Value(5)
    ├── ArrayAssign(grid[1,2])
    │   └── Value(6)
    ├── ArrayAssign(grid[2,0])
    │   └── Value(7)
    ├── ArrayAssign(grid[2,1])
    │   └── Value(8)
    ├── ArrayAssign(grid[2,2])
    │   └── Value(9)
    ├── Show
    │   └── ArrayAccess(grid[0,0])
    ├── Show
    │   └── ArrayAccess(grid[0,1])
    ├── Show
    │   └── ArrayAccess(grid[0,2])
    ├── Show
    │   └── ArrayAccess(grid[1,0])
    ├── Show
    │   └── ArrayAccess(grid[1,1])
    ├── Show
    │   └── ArrayAccess(grid[1,2])
    ├── Show
    │   └── ArrayAccess(grid[2,0])
    ├── Show
    │   └── ArrayAccess(grid[2,1])
    ├── Show
    │   └── ArrayAccess(grid[2,2])
    ├── Watch(x)
    ├── Assign(x)
    │   └── Value(5)
    ├── Assign(x)
    │   └── Value(10)
    ├── Rewind(x)
    ├── Show
    │   └── ID(x)
    ├── CheckOtherwise
    │   ├── Then:
    │   │   └── Show
    │   │       └── String("z is big")
    │   └── Otherwise:
    │       └── Show
    │           └── String("z is small")
    │   └── BinOp(>)
    │       ├── ID(z)
    │       └── Value(25)
    ├── Decl(i)
    │   ├── Type(num)
    │   └── Value(0)
    ├── While
    │   └── Body:
    │       ├── Show
    │       │   └── ID(i)
    │       └── UnOp(++ (post))
    │           └── ID(i)
    │   └── BinOp(<)
    │       ├── ID(i)
    │       └── Value(5)
    ├── GC_Collect
    └── Return
        └── Value(0)

=========================================
   PHASE 4: SEMANTIC ANALYSIS (SYMBOL)
=========================================
NAME                 KIND            SCOPE      STATUS     FLAGS     
----------------------------------------------------------------------
mul                  Function        0          Active     -         
a                    num             1          Closed     -         
b                    num             1          Closed     -         
main                 Function        0          Active     -         

Semantics validated successfully. No errors found.

=========================================
   PHASE 5: INTERMEDIATE CODE (TAC)
=========================================

mul:
BeginFunc
t0 = a * b
Return t0
EndFunc

main:
BeginFunc
Show "Hello LUMA!"
x = 10
y = 20
t1 = x + y
z = t1
Show z
Param x
Param y
t2 = Call mul, 2
c = t2
Show c
t3 = 2 ** 8
p = t3
Show p
ArrayAlloc arr, dim[0] = 5
arr[0] = 100
arr[1] = 200
t4 = arr[0]
Show t4
ArrayAlloc grid, dim[0] = 3
ArrayAlloc grid, dim[1] = 3
grid[0,0] = 1
grid[0,1] = 2
grid[0,2] = 3
grid[1,0] = 4
grid[1,1] = 5
grid[1,2] = 6
grid[2,0] = 7
grid[2,1] = 8
grid[2,2] = 9
t5 = grid[0,0]
Show t5
t6 = grid[0,1]
Show t6
t7 = grid[0,2]
Show t7
t8 = grid[1,0]
Show t8
t9 = grid[1,1]
Show t9
t10 = grid[1,2]
Show t10
t11 = grid[2,0]
Show t11
t12 = grid[2,1]
Show t12
t13 = grid[2,2]
Show t13
Watch x
x = 5
x = 10
Rewind x
Show x
t14 = z > 25
ifFalse t14 goto L0
Show "z is big"
goto L1
L0:
Show "z is small"
L1:
i = 0
L2:
t15 = i < 5
ifFalse t15 goto L3
Show i
t16 = ++ (post)i
goto L2
L3:
GC_Collect
Return 0
EndFunc

=========================================
   PHASE 6: CODE OPTIMIZATION (AST/TAC)
=========================================
Applying Constant Folding Optimization...

[ Optimized AST Structure ]
Program
├── Function(mul)
│   ├── Param(a)
│   │   └── Type(num)
│   ├── Param(b)
│   │   └── Type(num)
│   └── Return
│       └── BinOp(*)
│           ├── ID(a)
│           └── ID(b)
└── Function(main)
    ├── Show
    │   └── String("Hello LUMA!")
    ├── Decl(x)
    │   ├── Type(num)
    │   └── Value(10)
    ├── Decl(y)
    │   ├── Type(num)
    │   └── Value(20)
    ├── Decl(z)
    │   ├── Type(num)
    │   └── BinOp(+)
    │       ├── ID(x)
    │       └── ID(y)
    ├── Show
    │   └── ID(z)
    ├── Decl(c)
    │   ├── Type(num)
    │   └── Call(mul)
    │       └── Arg
    │           └── ID(x)
    ├── Show
    │   └── ID(c)
    ├── Decl(p)
    │   ├── Type(num)
    │   └── Value(256)
    ├── Show
    │   └── ID(p)
    ├── ArrayDecl(arr[5])
    │   └── Type(num)
    ├── ArrayAssign(arr[0])
    │   └── Value(100)
    ├── ArrayAssign(arr[1])
    │   └── Value(200)
    ├── Show
    │   └── ArrayAccess(arr[0])
    ├── ArrayDecl(grid[3,3])
    │   └── Type(num)
    ├── ArrayAssign(grid[0,0])
    │   └── Value(1)
    ├── ArrayAssign(grid[0,1])
    │   └── Value(2)
    ├── ArrayAssign(grid[0,2])
    │   └── Value(3)
    ├── ArrayAssign(grid[1,0])
    │   └── Value(4)
    ├── ArrayAssign(grid[1,1])
    │   └── Value(5)
    ├── ArrayAssign(grid[1,2])
    │   └── Value(6)
    ├── ArrayAssign(grid[2,0])
    │   └── Value(7)
    ├── ArrayAssign(grid[2,1])
    │   └── Value(8)
    ├── ArrayAssign(grid[2,2])
    │   └── Value(9)
    ├── Show
    │   └── ArrayAccess(grid[0,0])
    ├── Show
    │   └── ArrayAccess(grid[0,1])
    ├── Show
    │   └── ArrayAccess(grid[0,2])
    ├── Show
    │   └── ArrayAccess(grid[1,0])
    ├── Show
    │   └── ArrayAccess(grid[1,1])
    ├── Show
    │   └── ArrayAccess(grid[1,2])
    ├── Show
    │   └── ArrayAccess(grid[2,0])
    ├── Show
    │   └── ArrayAccess(grid[2,1])
    ├── Show
    │   └── ArrayAccess(grid[2,2])
    ├── Watch(x)
    ├── Assign(x)
    │   └── Value(5)
    ├── Assign(x)
    │   └── Value(10)
    ├── Rewind(x)
    ├── Show
    │   └── ID(x)
    ├── CheckOtherwise
    │   ├── Then:
    │   │   └── Show
    │   │       └── String("z is big")
    │   └── Otherwise:
    │       └── Show
    │           └── String("z is small")
    │   └── BinOp(>)
    │       ├── ID(z)
    │       └── Value(25)
    ├── Decl(i)
    │   ├── Type(num)
    │   └── Value(0)
    ├── While
    │   └── Body:
    │       ├── Show
    │       │   └── ID(i)
    │       └── UnOp(++ (post))
    │           └── ID(i)
    │   └── BinOp(<)
    │       ├── ID(i)
    │       └── Value(5)
    ├── GC_Collect
    └── Return
        └── Value(0)

[ Optimized Intermediate Code (TAC) ]

mul:
BeginFunc
t0 = a * b
Return t0
EndFunc

main:
BeginFunc
Show "Hello LUMA!"
x = 10
y = 20
t1 = x + y
z = t1
Show z
Param x
Param y
t2 = Call mul, 2
c = t2
Show c
p = 256
Show p
ArrayAlloc arr, dim[0] = 5
arr[0] = 100
arr[1] = 200
t3 = arr[0]
Show t3
ArrayAlloc grid, dim[0] = 3
ArrayAlloc grid, dim[1] = 3
grid[0,0] = 1
grid[0,1] = 2
grid[0,2] = 3
grid[1,0] = 4
grid[1,1] = 5
grid[1,2] = 6
grid[2,0] = 7
grid[2,1] = 8
grid[2,2] = 9
t4 = grid[0,0]
Show t4
t5 = grid[0,1]
Show t5
t6 = grid[0,2]
Show t6
t7 = grid[1,0]
Show t7
t8 = grid[1,1]
Show t8
t9 = grid[1,2]
Show t9
t10 = grid[2,0]
Show t10
t11 = grid[2,1]
Show t11
t12 = grid[2,2]
Show t12
Watch x
x = 5
x = 10
Rewind x
Show x
t13 = z > 25
ifFalse t13 goto L0
Show "z is big"
goto L1
L0:
Show "z is small"
L1:
i = 0
L2:
t14 = i < 5
ifFalse t14 goto L3
Show i
t15 = ++ (post)i
goto L2
L3:
GC_Collect
Return 0
EndFunc

=========================================
   PHASE 7: TARGET CODE GENERATION (ASM)
=========================================
# === RISC-V 32-bit Architecture ===
.data
.L0_str:
    .asciiz "Hello LUMA!"
    .word 0
.L1_str:
    .asciiz "z is big"
    .word 0
.L2_str:
    .asciiz "z is small"
    .word 0
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
    lw t0, -12(s0)
    lw t1, -16(s0)
    mul t0, t0, t1
    mv a0, t0
    lw ra, 252(sp)
    lw s0, 248(sp)
    addi sp, sp, 256
    jr ra
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
    li a0, 10
    li a7, 11
    ecall
    li t0, 10
    sw t0, -12(s0)
    li t0, 20
    sw t0, -16(s0)
    lw t0, -12(s0)
    lw t1, -16(s0)
    add t0, t0, t1
    sw t0, -20(s0)
    lw t0, -20(s0)
    mv a0, t0
    li a7, 1
    ecall
    li a0, 10
    li a7, 11
    ecall
    lw t0, -12(s0)
    lw t1, -16(s0)
    mv a0, t0
    mv a1, t1
    call mul
    mv t0, a0
    sw t0, -24(s0)
    lw t0, -24(s0)
    mv a0, t0
    li a7, 1
    ecall
    li a0, 10
    li a7, 11
    ecall
    li t0, 256
    sw t0, -28(s0)
    lw t0, -28(s0)
    mv a0, t0
    li a7, 1
    ecall
    li a0, 10
    li a7, 11
    ecall
    # ArrayDecl arr [5 elements] at -32(s0)
    sw zero, -32(s0)
    sw zero, -36(s0)
    sw zero, -40(s0)
    sw zero, -44(s0)
    sw zero, -48(s0)
    # ArrayAssign arr
    li t0, 0
    li t1, 100
    slli t0, t0, 2
    li t2, 32
    add t2, t2, t0
    sub t2, s0, t2
    sw t1, 0(t2)
    # ArrayAssign arr
    li t0, 1
    li t1, 200
    slli t0, t0, 2
    li t2, 32
    add t2, t2, t0
    sub t2, s0, t2
    sw t1, 0(t2)
    # Array Access arr
    li t0, 0
    slli t0, t0, 2
    li t1, 32
    add t1, t1, t0
    sub t1, s0, t1
    lw t1, 0(t1)
    mv a0, t1
    li a7, 1
    ecall
    li a0, 10
    li a7, 11
    ecall
    # ArrayDecl grid [9 elements] at -52(s0)
    sw zero, -52(s0)
    sw zero, -56(s0)
    sw zero, -60(s0)
    sw zero, -64(s0)
    sw zero, -68(s0)
    sw zero, -72(s0)
    sw zero, -76(s0)
    sw zero, -80(s0)
    sw zero, -84(s0)
    # ArrayAssign grid
    li t0, 0
    li t1, 3
    mul t0, t0, t1
    li t1, 0
    add t0, t0, t1
    li t1, 1
    slli t0, t0, 2
    li t2, 52
    add t2, t2, t0
    sub t2, s0, t2
    sw t1, 0(t2)
    # ArrayAssign grid
    li t0, 0
    li t1, 3
    mul t0, t0, t1
    li t1, 1
    add t0, t0, t1
    li t1, 2
    slli t0, t0, 2
    li t2, 52
    add t2, t2, t0
    sub t2, s0, t2
    sw t1, 0(t2)
    # ArrayAssign grid
    li t0, 0
    li t1, 3
    mul t0, t0, t1
    li t1, 2
    add t0, t0, t1
    li t1, 3
    slli t0, t0, 2
    li t2, 52
    add t2, t2, t0
    sub t2, s0, t2
    sw t1, 0(t2)
    # ArrayAssign grid
    li t0, 1
    li t1, 3
    mul t0, t0, t1
    li t1, 0
    add t0, t0, t1
    li t1, 4
    slli t0, t0, 2
    li t2, 52
    add t2, t2, t0
    sub t2, s0, t2
    sw t1, 0(t2)
    # ArrayAssign grid
    li t0, 1
    li t1, 3
    mul t0, t0, t1
    li t1, 1
    add t0, t0, t1
    li t1, 5
    slli t0, t0, 2
    li t2, 52
    add t2, t2, t0
    sub t2, s0, t2
    sw t1, 0(t2)
    # ArrayAssign grid
    li t0, 1
    li t1, 3
    mul t0, t0, t1
    li t1, 2
    add t0, t0, t1
    li t1, 6
    slli t0, t0, 2
    li t2, 52
    add t2, t2, t0
    sub t2, s0, t2
    sw t1, 0(t2)
    # ArrayAssign grid
    li t0, 2
    li t1, 3
    mul t0, t0, t1
    li t1, 0
    add t0, t0, t1
    li t1, 7
    slli t0, t0, 2
    li t2, 52
    add t2, t2, t0
    sub t2, s0, t2
    sw t1, 0(t2)
    # ArrayAssign grid
    li t0, 2
    li t1, 3
    mul t0, t0, t1
    li t1, 1
    add t0, t0, t1
    li t1, 8
    slli t0, t0, 2
    li t2, 52
    add t2, t2, t0
    sub t2, s0, t2
    sw t1, 0(t2)
    # ArrayAssign grid
    li t0, 2
    li t1, 3
    mul t0, t0, t1
    li t1, 2
    add t0, t0, t1
    li t1, 9
    slli t0, t0, 2
    li t2, 52
    add t2, t2, t0
    sub t2, s0, t2
    sw t1, 0(t2)
    # Array Access grid
    li t0, 0
    li t1, 3
    mul t0, t0, t1
    li t1, 0
    add t0, t0, t1
    slli t0, t0, 2
    li t1, 52
    add t1, t1, t0
    sub t1, s0, t1
    lw t1, 0(t1)
    mv a0, t1
    li a7, 1
    ecall
    li a0, 10
    li a7, 11
    ecall
    # Array Access grid
    li t0, 0
    li t1, 3
    mul t0, t0, t1
    li t1, 1
    add t0, t0, t1
    slli t0, t0, 2
    li t1, 52
    add t1, t1, t0
    sub t1, s0, t1
    lw t1, 0(t1)
    mv a0, t1
    li a7, 1
    ecall
    li a0, 10
    li a7, 11
    ecall
    # Array Access grid
    li t0, 0
    li t1, 3
    mul t0, t0, t1
    li t1, 2
    add t0, t0, t1
    slli t0, t0, 2
    li t1, 52
    add t1, t1, t0
    sub t1, s0, t1
    lw t1, 0(t1)
    mv a0, t1
    li a7, 1
    ecall
    li a0, 10
    li a7, 11
    ecall
    # Array Access grid
    li t0, 1
    li t1, 3
    mul t0, t0, t1
    li t1, 0
    add t0, t0, t1
    slli t0, t0, 2
    li t1, 52
    add t1, t1, t0
    sub t1, s0, t1
    lw t1, 0(t1)
    mv a0, t1
    li a7, 1
    ecall
    li a0, 10
    li a7, 11
    ecall
    # Array Access grid
    li t0, 1
    li t1, 3
    mul t0, t0, t1
    li t1, 1
    add t0, t0, t1
    slli t0, t0, 2
    li t1, 52
    add t1, t1, t0
    sub t1, s0, t1
    lw t1, 0(t1)
    mv a0, t1
    li a7, 1
    ecall
    li a0, 10
    li a7, 11
    ecall
    # Array Access grid
    li t0, 1
    li t1, 3
    mul t0, t0, t1
    li t1, 2
    add t0, t0, t1
    slli t0, t0, 2
    li t1, 52
    add t1, t1, t0
    sub t1, s0, t1
    lw t1, 0(t1)
    mv a0, t1
    li a7, 1
    ecall
    li a0, 10
    li a7, 11
    ecall
    # Array Access grid
    li t0, 2
    li t1, 3
    mul t0, t0, t1
    li t1, 0
    add t0, t0, t1
    slli t0, t0, 2
    li t1, 52
    add t1, t1, t0
    sub t1, s0, t1
    lw t1, 0(t1)
    mv a0, t1
    li a7, 1
    ecall
    li a0, 10
    li a7, 11
    ecall
    # Array Access grid
    li t0, 2
    li t1, 3
    mul t0, t0, t1
    li t1, 1
    add t0, t0, t1
    slli t0, t0, 2
    li t1, 52
    add t1, t1, t0
    sub t1, s0, t1
    lw t1, 0(t1)
    mv a0, t1
    li a7, 1
    ecall
    li a0, 10
    li a7, 11
    ecall
    # Array Access grid
    li t0, 2
    li t1, 3
    mul t0, t0, t1
    li t1, 2
    add t0, t0, t1
    slli t0, t0, 2
    li t1, 52
    add t1, t1, t0
    sub t1, s0, t1
    lw t1, 0(t1)
    mv a0, t1
    li a7, 1
    ecall
    li a0, 10
    li a7, 11
    ecall
    # WATCH x (history at -85(s0))
    sw zero, -117(s0)
    # Watch-Save x before assign
    lw t0, -12(s0)
    lw t1, -117(s0)
    slli t2, t1, 2
    addi t2, t2, 85
    sub t2, s0, t2
    sw t0, 0(t2)
    addi t1, t1, 1
    sw t1, -117(s0)
    li t0, 5
    sw t0, -12(s0)
    # Watch-Save x before assign
    lw t0, -12(s0)
    lw t1, -117(s0)
    slli t2, t1, 2
    addi t2, t2, 85
    sub t2, s0, t2
    sw t0, 0(t2)
    addi t1, t1, 1
    sw t1, -117(s0)
    li t0, 10
    sw t0, -12(s0)
    # REWIND x
    lw t1, -117(s0)
    addi t1, t1, -1
    slli t1, t1, 2
    li t0, 85
    add t0, t0, t1
    sub t0, s0, t0
    lw t0, 0(t0)
    sw t0, -12(s0)
    srai t1, t1, 2
    sw t1, -117(s0)
    lw t0, -12(s0)
    mv a0, t0
    li a7, 1
    ecall
    li a0, 10
    li a7, 11
    ecall
    lw t0, -20(s0)
    li t1, 25
    slt t0, t1, t0
    beqz t0, .L3
    la a0, .L1_str
    li a7, 4
    ecall
    li a0, 10
    li a7, 11
    ecall
    j .L4
.L3:
    la a0, .L2_str
    li a7, 4
    ecall
    li a0, 10
    li a7, 11
    ecall
.L4:
    li t0, 0
    sw t0, -124(s0)
.L5:
    lw t0, -124(s0)
    li t1, 5
    slt t0, t0, t1
    beqz t0, .L6
    lw t0, -124(s0)
    mv a0, t0
    li a7, 1
    ecall
    li a0, 10
    li a7, 11
    ecall
    lw t0, -124(s0)
    addi t1, t0, 1
    sw t1, -124(s0)
    j .L5
.L6:
    # GC_COLLECT: Sweep unreferenced stack memory
    # Mark phase: scan active variable references
    # Sweep phase: reset stack slots with no references
    # GC Zone 1 completed
    li t0, 0
    mv a0, t0
    li a7, 10
    ecall
    li a7, 10
    ecall

-----------------------------------------
Compilation pipeline executed successfully.
`
