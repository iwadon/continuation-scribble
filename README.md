# continuation-scribble

**English** | [日本語](README.ja.md)

Experimental project exploring low-level continuation and cooperative fiber (coroutine) implementation in C and assembly. Supports multiple CPU architectures.

## Supported Architectures

- **arm64**: Apple Silicon (macOS)
- **arm64-linux**: ARM64 (Linux)
- **x86_64**: Intel/AMD (macOS)
- **x86_64-linux**: Intel/AMD (Linux)
- **x86_64-win**: Intel/AMD (Windows, MSVC)
- **m68k**: Motorola 68000 (X68000, cross-compilation)

## Build Instructions

Each architecture has its own build configuration:

```bash
ninja -C arm64        # ARM64 (macOS Apple Silicon)
ninja -C arm64-linux  # ARM64 (Linux)
ninja -C x86_64       # x86_64 (macOS Intel)
ninja -C x86_64-linux # x86_64 (Linux)
ninja -C m68k-xelf    # m68k (X68000 cross-compilation)
```

### Windows (x86_64-win)

Requires Visual Studio with C++ workload installed.

```cmd
cd x86_64-win
build_msvc.bat          # Build x86_64_ctx_test.exe (default)
build_msvc.bat all      # Build all tests
build_msvc.bat clean    # Clean build artifacts
```

The build script automatically sets up the MSVC environment using `vcvarsall.bat`.

## Running Tests

### arm64 (native on Apple Silicon)

```bash
./build/arm64-apple/arm64_ctx_test
./build/arm64-apple/fiber_test
./build/arm64-apple/cont_test
./build/arm64-apple/cont_clone_test
```

### x86_64 (native or Rosetta 2)

```bash
./build/x86_64-apple/x86_64_ctx_test
./build/x86_64-apple/fiber_test
./build/x86_64-apple/cont_test
./build/x86_64-apple/cont_clone_test
```

### arm64-linux (Linux ARM64)

```bash
./build/arm64-linux/arm64_ctx_test
./build/arm64-linux/fiber_test
./build/arm64-linux/cont_test
./build/arm64-linux/cont_clone_test
```

### x86_64-linux (Linux x86_64)

```bash
./build/x86_64-linux/x86_64_ctx_test
./build/x86_64-linux/fiber_test
./build/x86_64-linux/cont_test
./build/x86_64-linux/cont_clone_test
```

### x86_64-win (Windows)

```cmd
build\x86_64-win\x86_64_ctx_test.exe
build\x86_64-win\fiber_test.exe
build\x86_64-win\cont_test.exe
build\x86_64-win\cont_clone_test.exe
```

### m68k (requires run68 emulator)

```bash
run68 ./build/m68k-xelf/m68k_ctx_test.x
run68 ./build/m68k-xelf/fiber_test.x
run68 ./build/m68k-xelf/cont_test.x
run68 ./build/m68k-xelf/cont_clone_test.x
```

## Layer Structure

### 1. Context Layer (lowest level)

Architecture-specific register save/restore. Similar to setjmp/longjmp semantics.

- `*_ctx_save()`: Returns 0 on save, non-zero when resumed
- `*_ctx_resume()`: Restores context (never returns)

### 2. Fiber Layer

Cooperative userspace threads with explicit yielding. Each fiber has its own stack.

### 3. Continuation Layer

Full continuation support with stack image capture/restore. Supports multi-shot continuations (can resume the same continuation multiple times).

## Using the Continuation Layer

### Basic API

```c
#include "cont.h"

cont_t cont;
cont_init(&cont);           // Initialize continuation structure

if (cont_save(&cont) == 0) {
    // First time: continuation saved
    // ... do something ...
    cont_resume(&cont);     // Jump back to cont_save point
}
// Resumed here (cont_save returns 1)

cont_term(&cont);           // Clean up
```

### Multi-shot Continuations

```c
cont_t base, c1, c2;
cont_init(&base);
cont_init(&c1);
cont_init(&c2);

if (cont_save(&base) == 0) {
    cont_clone(&c1, &base);  // Copy continuation
    cont_clone(&c2, &base);  // Can clone multiple times
    cont_resume(&c1);        // Resume first copy
}
// Can resume c2 later for multi-shot behavior
```

### Important Usage Notes

#### 1. Store continuations outside the captured stack

Continuation structures must be stored in static/global memory or on the heap, NOT on the stack being captured. Otherwise, resuming will corrupt the continuation structure itself.

```c
// WRONG: cont is on the stack being captured
void bad_example(void) {
    cont_t cont;  // Stack variable - will be corrupted!
    cont_init(&cont);
    if (cont_save(&cont) == 0) {
        cont_resume(&cont);  // Undefined behavior
    }
}

// CORRECT: cont is in static memory
static cont_t cont;
void good_example(void) {
    cont_init(&cont);
    if (cont_save(&cont) == 0) {
        cont_resume(&cont);  // Works correctly
    }
}
```

#### 2. Set cont_stack_base before cont_save

The `cont_stack_base` global must be set to mark the top of the stack region to capture. Set it at the entry point, before calling any functions that use continuations.

```c
char *cont_stack_base;

static void get_stack_base(void) {
#if defined(__aarch64__)
    asm volatile("mov %0, sp" : "=r"(cont_stack_base));
#elif defined(__human68k__)
    asm volatile("move.l %%a7, %0" : "=r"(cont_stack_base));
#elif defined(_MSC_VER) && defined(_WIN64)
    cont_stack_base = (char *)_AddressOfReturnAddress();
#elif defined(__x86_64__)
    asm volatile("movq %%rsp, %0" : "=r"(cont_stack_base));
#endif
}

int main(void) {
    get_stack_base();  // Set stack base FIRST
    run_application(); // Then call your code
    return 0;
}
```

#### 3. Use nested functions for proper stack capture

The stack region captured is from `cont_stack_base` down to the SP at `cont_save` time. To capture meaningful state, ensure there's a function call between setting `cont_stack_base` and calling `cont_save`.

```c
static cont_t cont;

static void do_work(void) {
    // This function's stack frame will be captured
    if (cont_save(&cont) == 0) {
        // ...
        cont_resume(&cont);
    }
}

int main(void) {
    get_stack_base();
    do_work();  // Stack frame of do_work is captured
    return 0;
}
```

### Error Handling

Set `cont_panic_fn` to handle errors:

```c
void my_panic(const char *msg) {
    fprintf(stderr, "Continuation error: %s\n", msg);
}

int main(void) {
    cont_panic_fn = my_panic;
    // ...
}
```

## License

This project is licensed under MIT No Attribution (MIT-0). See [LICENSE](LICENSE) for details.
