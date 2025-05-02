# COMP475: 2D Graphics Engine

A minimal 2D vector‑graphics engine for COMP475, with automated test‑image generation and regression checking.

---

## Build

```bash
make
# (or, if you’ve installed CMake)
# mkdir build && cd build && cmake .. && make
```

---

## Usage

```bash
./image [options]
```

**Options**

- `-e` render & diff against `expected/`  
- `-v` verbose logs  
- `-d DIR` dump all frames + HTML gallery to `DIR`  
- `-m LIST` run only named/indexed tests (comma‑separated)  

---

## Repo Layout

```text
include/      public headers
src/          engine implementation
apps/         sample apps & tests
expected/     golden PNGs
Makefile      build rules
```
