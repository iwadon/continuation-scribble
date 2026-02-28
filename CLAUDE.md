# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Low-level continuation and cooperative fiber (coroutine) library written in C and assembly. Supports multiple CPU architectures for context switching primitives.

## Build Commands

Each architecture has its own ninja build configuration in its directory:

```bash
ninja -C {arch-dir}   # e.g. ninja -C arm64-mac
```

See README.md for the full list of supported architectures and test commands. Test binaries are output to `build/{arch}/`.

## Architecture

### Layer Structure

The library is organized in layers of increasing abstraction. See README.md for an overview.

1. **Context Layer** (`*_ctx.h`, `*_ctx.s`) - Architecture-specific register save/restore:
   - `*_ctx_save()` returns 0 on save, non-zero when resumed (setjmp/longjmp semantics)
   - Each arch defines its own `*_ctx_t` struct for register storage
   - Entry point and argument are passed via callee-saved registers — see `fiber.c:fiber_create()` and the corresponding context header (e.g., `arm64-mac/arm64_ctx.h`, `x86_64-mac/x86_64_ctx.h`)

2. **Fiber Layer** (`fiber.h`, `fiber.c`, `fiber_trampoline.s`):
   - Cooperative userspace threads with explicit yielding
   - Each fiber has own stack; main fiber has no allocated stack
   - `fiber_trampoline` (assembly) handles first entry into fiber

3. **Scheduler Layer** (`scheduler.h`, `scheduler.c`):
   - Cooperative task scheduler for managing fiber execution
   - See header file for public API

4. **Continuation Layer** (`cont.h`, `cont.c`):
   - Full continuation support with stack image capture/restore
   - See README.md for usage patterns and important constraints

5. **Effect Handler Layer** (`eff.h`, `eff.c`):
   - Algebraic effects using delimited continuations
   - `EFF_HANDLE`/`EFF_WITH`/`EFF_END` macros for structured effect handling
   - `eff_perform()` suspends computation; `eff_resume()` resumes captured continuation

### Architecture Directory Convention

Each architecture has a directory named `{arch}-{platform}` (e.g., `arm64-mac`, `x86_64-linux`, `m68k-xelf`). All share the same source files (`fiber.c`, `scheduler.c`, etc.) but have architecture-specific context and trampoline assembly.

## Code Style

- Formatting rules defined in `.clang-format` (WebKit-based) and `.editorconfig`
- C files: tabs with 4-space display width
- Assembly files (`.s`): tabs with 8-space display width
