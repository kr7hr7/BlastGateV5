# Functional Specification Document — pre-commit Git Hook

**Project:** BlastGateV5  
**Hook file:** `.githooks/pre-commit`  
**Supporting script:** `scripts/update_ver_date.ps1`  
**Date:** 03_20_2026  

---

## 1. Purpose

Automatically keep the firmware version date string (`ver`) in sync with the calendar date of each commit. This ensures the date embedded in the firmware binary always reflects when the firmware was last committed, with no manual intervention required.

---

## 2. Trigger

The hook executes automatically as part of every `git commit` operation, immediately before Git writes the commit object. It runs in the working tree of the repository on the developer's Windows machine.

---

## 3. Files Involved

| File | Role |
|---|---|
| `.githooks/pre-commit` | Shell entry point; invoked by Git |
| `scripts/update_ver_date.ps1` | PowerShell worker; performs the version update |
| `src/globals.cpp` | Target source file containing the `ver` variable |
| `include/globals.h` | Declares `extern const char* ver` — consumed throughout the firmware |

---

## 4. Functional Flow

```
git commit
    │
    └─► .githooks/pre-commit (sh)
            │
            ├─ Resolves repository root via: git rev-parse --show-toplevel
            │
            └─► scripts/update_ver_date.ps1 (PowerShell)
                    │
                    ├─ Locates src/globals.cpp relative to repo root
                    ├─ Reads file content into memory
                    ├─ Gets today's date in MM_dd_yyyy format
                    ├─ Matches regex pattern:
                    │      const char* ver = "MM_DD_YYYY";
                    ├─ Replaces matched line with today's date
                    ├─ Writes updated file back to disk (UTF-8, no BOM)
                    ├─ Runs: git add -- src/globals.cpp
                    └─ Reports success/failure to stdout/stderr
```

---

## 5. Detailed Behavior

### 5.1 Shell Hook (`pre-commit`)

- Uses `#!/usr/bin/env sh` for POSIX portability.
- `set -eu` — exits immediately on any error (`-e`) and treats unset variables as errors (`-u`).
- Resolves `repo_root` dynamically so the hook works regardless of the current working directory within the repository.
- Invokes PowerShell with `-NoProfile` (faster startup) and `-ExecutionPolicy Bypass` (no machine policy required).

### 5.2 PowerShell Script (`update_ver_date.ps1`)

| Step | Detail |
|---|---|
| **Input parameter** | `$TargetFile` defaults to `src/globals.cpp`; overridable for testing. |
| **File existence check** | Exits with code 1 and a descriptive error if the file is not found. |
| **Date format** | `Get-Date -Format "MM_dd_yyyy"` — zero-padded month and day, 4-digit year. |
| **Regex pattern** | `(?m)^const\s+char\*\s+ver\s*=\s*"\d{2}_\d{2}_\d{4}";.*$` — matches the entire `ver` assignment line, tolerates whitespace variation. |
| **Replacement** | `const char* ver = "MM_dd_yyyy";  // MM_DD_YYYY` |
| **Pattern not found** | Exits with code 1 — commit is aborted, preventing a silent version mismatch. |
| **File write** | `[System.IO.File]::WriteAllText(...)` with explicit UTF-8 (no BOM) encoding to preserve file consistency. |
| **Stage the change** | `git add -- src/globals.cpp` — ensures the updated date is included in the current commit automatically. |

---

## 6. Version String Format

```
MM_dd_yyyy
```

Examples: `03_20_2026`, `12_01_2025`

The string is declared in `src/globals.cpp`:

```cpp
const char* ver = "03_20_2026";  // MM_DD_YYYY
```

And declared extern in `include/globals.h`:

```cpp
extern const char* ver;    // firmware version string
```

---

## 7. Error Conditions

| Condition | Behaviour |
|---|---|
| `src/globals.cpp` does not exist | Script exits 1, hook exits 1, commit aborted |
| `ver` assignment pattern not found in file | Script exits 1, hook exits 1, commit aborted |
| PowerShell not available | Shell hook exits non-zero, commit aborted |
| Git hook not enabled (hook not in `.git/hooks/`) | Hook does not run; version not updated |

---

## 8. Setup Requirement

Git does not automatically use `.githooks/`. The developer must configure Git to find the hooks directory once per clone:

```sh
git config core.hooksPath .githooks
```

---

## 9. Scope and Limitations

- Runs only on the committing developer's machine; CI/CD pipelines must run the script independently if the same guarantee is needed there.
- Updates the date but not a build number or semantic version; version differentiation between same-day commits is not supported.
- Only `src/globals.cpp` is patched; any other copy of the version string in the codebase is not touched.
