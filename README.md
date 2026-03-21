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
