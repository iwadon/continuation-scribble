## Build instructions with m68k-xelf-gcc (elf2x68k)

Build: `ninja -C m68k-xelf `
Run context test for m68k-xelf: `./build/m68k-xelf/m68k_ctx_test.x`
Run fiber test: `./build/m68k-xelf/fiber_test.x`

## Build instructions with arm64-apple-darwin (macOS)

Build: `ninja -C arm64`
Run context test for arm64: `./build/arm64-apple/m68k_ctx_test`
Run fiber test: `./build/arm64-apple/fiber_test`
