# xommit — Code Navigation Map

The code *is* the documentation. This file only gives you mental models to find things fast.

---

## Repository Layout

```
xommit/
├── .github/workflows/release.yml    # CI: build .exe → GitHub Release → npm publish
├── cli/                              # The entire product lives here
│   ├── package.json                  # Version source of truth
│   ├── src/                          # C++ implementation (18 files)
│   ├── include/xommit/               # Public headers
│   ├── scripts/
│   │   ├── build.js                  # Builds bin/xommit.exe via g++
│   │   ├── index.js                  # npm shim (entry point, downloads .exe)
│   │   ├── install.ps1               # Windows installer (PowerShell)
│   │   └── release.js                # Local tag + push (triggers CI)
│   └── bin/                          # Build output (gitignored)
├── docs/NAVIGATION.md                # This file
└── notes/                            # Design notes (optional reading)
```

---

## Mental Model 1: The Three Install Routes

All three end up with the same binary at `%LOCALAPPDATA%\xommit\bin\xommit.exe`

| Route | Entry Point | How It Gets the .exe |
|-------|-------------|----------------------|
| **npm** | `cli/scripts/index.js` | Shim downloads from GitHub Releases on first run |
| **PowerShell** | `cli/scripts/install.ps1` | Direct download from GitHub Releases |
| **Source** | `cli/scripts/build.js` | Compiles locally with MinGW |

**Key insight**: The `.exe` is *never* in the npm package. It lives in GitHub Releases. The npm package is just a Node shim + installer script.

---

## Mental Model 2: Commit Flow (What `xommit` Actually Does)

```
xommit [hint]
  │
  ├─ git rev-parse --show-toplevel          → repo root
  ├─ git diff --cached                      → staged diff only
  ├─ parse diff → file list + hunks
  ├─ filter with ignore rules (built-in + .xommitignore)
  ├─ load config (%APPDATA%\xommit\config.json)
  ├─ budget context (water-fill across files)
  ├─ build prompt (system + hint + diff)
  ├─ call provider (Google / Vercel) with retry
  ├─ parse 3 commit messages (Conventional Commits)
  ├─ show picker (arrow keys, type-to-filter)
  └─ git commit -m "<chosen>"
```

**Files to read**: `src/main.cpp` → `src/commit_flow.cpp` → `src/ai_client.cpp`

---

## Mental Model 3: Provider Abstraction

```
src/ai_client.h         → interface (AiClient, ModelInfo, ProviderConfig)
    │
    ├─ src/google_client.cpp   → Google AI Studio (generativelanguage.googleapis.com)
    └─ src/vercel_client.cpp   → Vercel AI Gateway (ai-gateway.vercel.sh)
```

Both implement the same virtual methods. Ranking logic in `src/provider_registry.cpp`.

---

## Mental Model 4: Configuration & State

| Location | Purpose |
|----------|---------|
| `%APPDATA%\xommit\config.json` | Provider, API key, model_id |
| `%LOCALAPPDATA%\xommit\bin\xommit.exe` | The binary |
| `%LOCALAPPDATA%\xommit\bin\version.txt` | Installed version (for update checks) |
| `%LOCALAPPDATA%\xommit\bin\xommit.old.exe` | Previous version (cleanup on next start) |
| `.xommitignore` (repo root) | Project-specific ignore patterns |

---

## Mental Model 5: Release Pipeline

```
You:  edit cli/package.json → git tag v1.0.4 → git push --tags
CI:   windows-latest
      ├─ Checkout + MSYS2 (MinGW) + Node 24
      ├─ npm install + npm run build        → bin/xommit.exe
      ├─ Verify tag == v + package.json version
      ├─ GitHub Release (asset: xommit.exe)
      └─ npm publish --provenance (OIDC, no token)
```

**Version guard**: Prevents `package.json: 1.0.3` + `tag: v1.0.4` mismatch.

---

## Mental Model 6: Build System

```
cli/scripts/build.js
  └─ spawns g++ with:
      -std=c++20 -O2 -static -municode
      -Iinclude src/*.cpp
      -o bin/xommit.exe
      -DXOMMIT_VERSION="<from package.json>"
```

No CMake, no Make. Single Node script drives the compile.

---

## Where to Start Reading

| Goal | Start Here |
|------|------------|
| Understand the commit flow | `cli/src/main.cpp` → `cli/src/commit_flow.cpp` |
| Add a provider | `cli/src/ai_client.h` → `cli/src/google_client.cpp` |
| Change the prompt | `cli/src/prompt_builder.cpp` |
| Modify ignore rules | `cli/src/ignore_rules.cpp` |
| Fix the picker UI | `cli/src/picker.cpp` |
| Debug the build | `cli/scripts/build.js` |
| Understand the npm shim | `cli/scripts/index.js` |
| Understand the installer | `cli/scripts/install.ps1` |
| Understand the release | `.github/workflows/release.yml` |

---

## Notes Folder (Optional Deep Dives)

```
notes/
├── architecture.md      # Module map, data flow
├── prompt.md            # Prompt construction, parsing, validation
├── providers.md         # Provider APIs, ranking, retries
├── context.md           # Context budgeting, water-fill
├── install.md           # Install routes, update flow
├── publishing.md        # Release checklist, npm OIDC
└── build_plan.md        # This file's source of truth
```