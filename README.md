# Custom Compiler Implementation

This is a personal/academic project to design and implement a custom compiler using **Flex** (Lexical Analyzer) and **Bison** (Syntax Analyzer), ultimately targeting Assembly. It follows the standard phases of compiler design.

---

## 🚀 Current Status: Phase 1 (Lexical Analysis)

Currently, the **Lexer** is built and tested independently using Flex. It acts as the "Scanner", reading raw source code character-by-character and grouping them into meaningful "Tokens", stripping away useless information like spaces, tabs, and newlines.

### Supported Tokens (Phase 1)
The Lexer currently recognizes the following fundamental grammar constructs:
*   **Keywords:** `int`, `if`, `else`
*   **Data Types:** `Identifiers` (e.g., variable names starting with letters), `Numbers` (e.g., integers)
*   **Arithmetic Operators:** `+`, `-`, `*`, `/`, `=`
*   **Punctuation/Symbols:** `(`, `)`, `{`, `}`, `;`

### Project Structure (Phase 1)
| File | Description |
| ---- | ----------- |
| `lexer.l` | The Flex source code. Contains the Token definitions (`T_INT`, `T_ID`), regex patterns, and a standalone `main()` loop to test Lexing independently. |
| `test.txt` | A sample piece of source code containing a variable assignment and an if/else block used to test the Lexer. |
| `implementation_plan.md` | My comprehensive roadmap defining all 8 phases of compiler design (from Scanning to Assembly generation). |

---

## 💻 How to Build and Run the Lexer (Windows)

Because we are compiling the Flex `.l` file manually for testing Phase 1, you will need tools like **MinGW** or **MSYS2** (which include `flex` and `gcc`) installed on your Windows machine.

**1. Generate the C code from the Flex file**
```bash
flex lexer.l
```
*(This generates a file named `lex.yy.c` in the directory)*

**2. Compile the generated C code into an executable**
```bash
gcc lex.yy.c -o lexer.exe
```

**3. Run the Lexer against the sample test code**
```bash
.\lexer.exe test.txt
```

### Example Test File (`test.txt`)
```c
int main() {
    int x = 5;
    float y = 3.14;

    // This is a test comment
    if (x == 5) {
        y = y + 1;
    }

    return 0;
}
```

### Expected Output
```text
Found keyword: int
Found identifier: main
Found symbol: (
Found symbol: )
Found symbol: {
Found keyword: int
...
Found symbol: }
```
*(Note: As we expand the language grammar, unsupported characters (like floating point `.` in this test) will appropriately throw unrecognized character warnings via the lexer catch-all dot rule).*

---

## 🔜 Next Steps
1. Push Phase 1 (Lexer) to version control.
2. Remove standalone testing code from `lexer.l`.
3. Introduce **Bison** (`parser.y`) for **Phase 2: Syntax Analysis/Parsing**, linking the two phases together.
