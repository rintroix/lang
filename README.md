1. `packcc` is built from external directory
2. `packcc` generates `parser.c` and `parser.h` from `parser.peg`
3. build `parser.c` and `main.c`

- `parser.c` includes `common.h`
- `main.c` includes generated `parser.h`
