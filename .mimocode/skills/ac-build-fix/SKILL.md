---
name: ac-build-fix
description: "Use when editing AzerothCore C++ source files and needing to compile, check errors, and iterate — the edit-compile-fix loop for worldserver/server builds"
---

# AzerothCore Build-Fix-Rebuild Loop

## Overview

This skill packages the dominant repeated workflow in AzerothCore development: edit C++ source → build worldserver → extract errors → fix → rebuild. It replaces the manual pattern of running cmake build commands with ad-hoc grep/tail variations.

## When to Use

- After editing any `.cpp` / `.h` file under `src/server/`
- When iterating on compilation errors
- When adding new features that require build verification
- Any time you need to compile the server to verify changes

## Build Paths (adjust if needed)

```
SOURCE_DIR: D:\1.PlusOrigin\LWCorePlus2026Edition
BUILD_DIR:  D:\1.PlusOrigin\LWCorePlus2026EditionBuild
TARGET:     worldserver
CONFIG:     RelWithDebInfo
PARALLEL:   -j8
```

## Workflow

### Step 1: Configure (only when CMakeLists.txt changed)

```bash
cd "<BUILD_DIR>" && cmake "<SOURCE_DIR>" -DSCRIPTS=static -DMODULES=static 2>&1 | tail -5
```

### Step 2: Build and capture errors

```bash
cd "<BUILD_DIR>" && cmake --build . --config <CONFIG> --target <TARGET> <PARALLEL> 2>&1 | grep -iE "error|fatal" | head -30
```

### Step 3: If errors found, read the error lines

Extract file paths and line numbers from error output, then Read the relevant source sections.

### Step 4: Fix with Edit

Apply the minimal fix to the identified source file(s).

### Step 5: Rebuild

Repeat from Step 2 until clean (zero errors).

### Step 6: Verify success

```bash
cd "<BUILD_DIR>" && cmake --build . --config <CONFIG> --target <TARGET> <PARALLEL> 2>&1 | tail -3
```

Confirm output shows "Build succeeded" or "0 errors".

## Error Triage Patterns

| Error pattern | Likely cause | Fix direction |
|---|---|---|
| `C2664: cannot convert argument` | Wrong type in function call | Check function signature, add cast or change type |
| `C2065: undeclared identifier` | Missing include or typo | Add #include or fix name |
| `E0135: class has no member` | API change or wrong class | Check upstream API, may need rebase |
| `LNK2019: unresolved external` | Missing library/linkage | Check CMakeLists.txt or missing .cpp in build |
| `C2039: is not a member of` | Namespace or class mismatch | Check using declarations and includes |

## MySQL Quick Reference

After a successful build, apply SQL updates:

```bash
mysql -u acore -pacore -h 127.0.0.1 -P 3308 acore_world < "<BUILD_DIR>/bin/<CONFIG>/MySqlUpdate/..."
```

Check creature/loot data:

```bash
mysql -u acore -pacore -h 127.0.0.1 -P 3308 acore_world -N -e "SELECT entry, name FROM creature_template WHERE entry = <ID>"
```

## Tips

- Always check errors first — don't blindly re-edit. The error message tells you exactly what's wrong.
- If the build hangs or shows MAPSTALL, that's a runtime issue, not a build issue — investigate separately.
- Use `grep -c "error"` to quickly count errors and track progress across iterations.
- When adding new scripts, also update `custom_script_loader.cpp` to register them.
