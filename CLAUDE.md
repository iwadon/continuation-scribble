# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Low-level continuation and cooperative fiber (coroutine) library written in C and assembly. Supports multiple CPU architectures for context switching primitives.

## Build Commands

Each architecture has its own ninja build configuration in its directory:

```bash
ninja -C arm64      # ARM64 (macOS Apple Silicon)
ninja -C arm64-win  # ARM64 (Windows on ARM)
ninja -C x86_64     # x86_64 (macOS Intel)
ninja -C m68k-xelf  # m68k (X68000 cross-compilation)
```

Test binaries are output to `build/{arch}/`. See README.md for specific test commands.

## Architecture

### Layer Structure

1. **Context Layer** (`*_ctx.h`, `*_ctx.s`) - Lowest level, architecture-specific:
   - Save/resume CPU register state (setjmp/longjmp-like semantics)
   - `*_ctx_save()` returns 0 on save, non-zero when resumed
   - Each arch defines its own `*_ctx_t` struct for register storage

2. **Fiber Layer** (`fiber.h`, `fiber.c`, `fiber_trampoline.s`):
   - Cooperative userspace threads with explicit yielding
   - Each fiber has own stack; main fiber has no allocated stack
   - `fiber_trampoline` (assembly) handles first entry into fiber

3. **Scheduler Layer** (`scheduler.h`, `scheduler.c`):
   - Cooperative task scheduler for managing fiber execution
   - See header file for public API

4. **Continuation Layer** (`cont.h`, `cont.c`):
   - Full continuation support with stack image capture/restore
   - Currently implemented for m68k architecture

### Architecture-Specific Directories

- `arm64/` - ARM64 Apple Silicon
- `arm64-win/` - ARM64 Windows (MSVC/armasm64)
- `x86_64/` - x86_64 Intel/AMD
- `m68k-xelf/` - Motorola 68000 (X68000), includes continuation support

### Fiber Entry Calling Convention

Entry point and argument are passed via callee-saved registers. See `fiber.c:fiber_create()` and arch-specific `*_ctx.h` for register assignments.

## Code Style

- Uses `.clang-format` (WebKit-based, tabs, 4-space width)
- Assembly files (`.s`) use 8-space tabs per `.editorconfig`
