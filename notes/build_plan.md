# xommit Build Plan

## Overview
A Windows-only C++ CLI that turns a vague message + staged git diff into 3 conventional commit options, then runs `git commit -m`. Single binary, no dependencies, distributed via npm shim + direct installers.

---

## Core Flow

```
xommit [hint...]
  ├─ no args / hint → commit flow
  ├─ --connect      → provider + key + model setup
  ├─ --reset        → delete config
  ├─ --update       → self-update from GitHub Releases
  ├─ --version      → print version
  └─ --help         → usage
```

### Commit flow
1. `git rev-parse --show-toplevel` → repo root
2. `git diff --cached` → staged diff only
3. If empty → `git commit` (no args), show git output, exit with git's code
4. Parse diff → file list with paths and hunks
5. Filter with ignore rules (built-in + `.xommitignore`)
6. If all filtered → "only ignored files are staged, nothing to send to the AI", exit 0
6. Load config (`%APPDATA%\xommit\config.json`) → provider, api_key, model
7. Budget context: `context_window - 512_output - prompt_overhead`, water-fill across files
8. Build prompt: system + user hint + diff context
9. Call provider (with one retry on 5xx/network)
10. Parse 3 commit messages (subject-only, ≤72 chars, Conventional Commits)
11. Show picker (arrow keys, type-to-filter, 10-row viewport, Esc=cancel)
12. `git commit -m "<chosen>"` → show git output

---

## Config

**Location**: `%APPDATA%\xommit\config.json`

```json
{
  "provider": "google",
  "api_key": "...",
  "model": {
    "id": "gemini-2.5-flash",
    "name": "Gemini 2.5 Flash",
    "context_window": 1048576,
    "max_output_tokens": 8192,
    "free": true
  }
}
```

- `--connect` overwrites entire config
- `--reset` deletes the file
- No environment variable fallback

---

## Ignore Rules

**Built-in patterns** (lockfiles, build output, dependencies, binary assets, minified files)

**Optional**: `.xommitignore` at repo root
- `# comment`, blank lines ignored
- `!pattern` negates
- `dir/` matches directory at any depth
- `path/to/file` globbed against full path
- `basename` globbed against filename or any path segment
- `*` and `?` supported

Rules only affect what is sent to AI. Staged files are still committed.

---

## Providers

### Google AI Studio (recommended first)
- Endpoint: `https://generativelanguage.googleapis.com/v1beta`
- Auth: `x-goog-api-key` header
- List: `GET /models?pageSize=1000` → filter `generateContent`, exclude embedding/aqa
- Generate: `POST /models/{id}:generateContent` with `systemInstruction`, `contents`, `generationConfig`
- Response: `candidates[0].content.parts[].text`

### Vercel AI Gateway
- Endpoint: `https://ai-gateway.vercel.sh/v1`
- Auth: `Authorization: Bearer`
- List: `GET /models` → filter `type=language`, output modality includes `text`
- Generate: `POST /chat/completions` with `messages[]`, `temperature`, `max_tokens`
- Response: `choices[0].message.content` (string or parts array)

### Ranking
| Provider | Priority |
|---|---|
| Google | flash-lite → flash → pro → others; penalize preview/exp/tts/audio/image/robotics |
| Vercel | free first → lowest combined price → largest context |

First model marked "Recommended" in picker.

### Output limit
512 tokens requested, or model's `max_output_tokens` if lower. Same value reserved in context budget.

### Errors
| Status | Message | Retry |
|---|---|---|
| 401 | API key rejected, run `--connect` | no |
| 403 | Access denied + body | no |
| 429 | Rate limit/quota + body | no |
| 5xx | Provider server error + body | once |
| other 4xx | Provider error + body | no |
| Network/timeout | WinHTTP error | once |

Timeouts: 10s connect, 30s send, 60s receive.

---

## UI

- **Logo**: 6-line ASCII from `salvage/terminal_ui.h`, shown only when terminal ≥80 cols
- **Spinner**: 4-frame dash animation on single line, clears on stop; non-TTY prints once
- **Password input**: `_getch`, echoes `*`, backspace works, Esc=cancel, Ctrl+C=exit
- **Picker**: 10-row viewport, type-to-filter, arrows, PgUp/PgDn, Home/End, Enter, Esc
- **Non-TTY fallback**: Numbered list + `getline`, invalid=cancel
- **Colors**: ANSI, disabled when not a TTY

---

## Git Interaction

- **Reads**: `CreateProcessW` with pipes + reader threads (no deadlock on large diffs), `CREATE_NO_WINDOW`
- **Writes**: `_wspawnvp` with inherited handles (git shows normal output, hooks, editor)
- **Commit message**: UTF-16 via `git commit -m` (non-ASCII survives)

---

## Build

**Toolchain**: MinGW g++ (GCC 16.2.0 UCRT64) from PATH (`D:\mingw64\bin`)

**Command**:
```bash
g++ -std=c++20 -O2 -Wall -Wextra -municode -DUNICODE -D_UNICODE \
  -DXOMMIT_VERSION="0.1.0" \
  -DXOMMIT_REPO="abhraneeldhar7/xommit" \
  -DXOMMIT_ASSET="xommit.exe" \
  -Iinclude src/*.cpp -o bin/xommit.exe \
  -static -static-libgcc -static-libstdc++ -lwinhttp
```

**Version source**: `cli/package.json` (single source of truth)

**Build script**: `cli/scripts/build.js` reads package.json, runs g++

---

## Install Routes (independent, documented separately)

### 1. npm global shim
```
npm install -g @abhraneeldhar7/xommit
```
- Package contains only `scripts/index.js`, `scripts/install.ps1`, `scripts/install.sh`, `README.md`
- Shim downloads pinned binary to `%LOCALAPPDATA%\xommit\bin` on first run (or if older)
- Never downgrades a newer `--update` binary
- Spawns exe with inherited stdio

### 2. PowerShell
```powershell
irm https://raw.githubusercontent.com/abhraneeldhar7/xommit/master/cli/scripts/install.ps1 | iex
# or pinned: .\install.ps1 -Version 0.1.0
```
- Downloads to `%LOCALAPPDATA%\xommit\bin`, adds to user PATH

### 3. Git Bash
```bash
curl -fsSL https://raw.githubusercontent.com/abhraneeldhar7/xommit/master/cli/scripts/install.sh | bash
```
- Same, appends PATH to `~/.bashrc`

---

## Update

```
xommit --update
```
1. `GET /repos/abhraneeldhar7/xommit/releases/latest` → latest tag
2. Compare with compiled-in version
3. Download `xommit.exe` to `xommit.new.exe`
4. Rename current → `xommit.old.exe`, move new into place, write `version.txt`
5. Next start deletes `xommit.old.exe`

---

## Publishing

- **Tag format**: `v0.1.0` (matches `cli/package.json` version)
- **Workflow**: `.github/workflows/release.yml` on `v*` tags, `windows-latest`
- **Steps**: version guard → build → GitHub Release (asset: `xommit.exe`) → npm OIDC publish
- **npm trusted publishing**: OIDC, no token, provenance enabled
- **Release script**: `cli/scripts/release.js` (clean tree check, tag exists check, build, tag, push)

---

## Repository Structure

```
xommit/
├── .github/workflows/release.yml
├── AGENTS.md                    # root only
├── .gitignore
├── cli/
│   ├── package.json             # version source
│   ├── README.md                # npm landing page
│   ├── include/xommit/          # 18 headers
│   ├── src/                     # 18 .cpp files
│   ├── scripts/
│   │   ├── build.js
│   │   ├── index.js             # npm shim
│   │   ├── install.ps1
│   │   ├── install.sh
│   │   └── release.js
│   └── bin/                     # gitignored, build output
├── notes/
│   ├── build_plan.md            # this file
│   ├── idea.md                  # original notes
│   ├── ai_provider_conversation.md
│   ├── install.md
│   ├── publishing.md
│   ├── providers.md
│   ├── prompt.md
│   └── architecture.md
├── salvage/
│   ├── terminal_ui.h            # logo source
│   └── LogoSpace.tsx
└── webapp/                      # untouched
```

---

## Code Organization (C++)

| File | Responsibility |
|---|---|
| `main.cpp` | `wmain`, UTF-8, VT enable, top-level catch |
| `cli.cpp` | Arg parsing, commit/connect/reset/update flows |
| `diff.cpp` | `git diff --cached` parsing, water-fill context budget |
| `ignore.cpp` | Built-in + `.xommitignore` matching |
| `git.cpp` | `git diff`, `git commit`, repo root |
| `prompt.cpp` | System prompt, user prompt builder, response parser |
| `provider.cpp` | Provider factory, ranking, formatting, error mapping |
| `google.cpp` | Google AI Studio API |
| `vercel.cpp` | Vercel AI Gateway API |
| `http.cpp` | WinHTTP with timeouts |
| `config.cpp` | `%APPDATA%\xommit\config.json` load/save/reset |
| `update.cpp` | GitHub latest release, download, binary swap |
| `json.cpp` | Hand-written recursive descent parser/serializer |
| `ui.cpp` | Logo, colors, password input, cursor, line clearing |
| `select.cpp` | Interactive picker + numbered fallback |
| `spinner.cpp` | Dash animation |
| `util.cpp` | Strings, paths, env, FS, UTF-8/16 conversion |
| `model.h` | `ModelSpec` struct |

---

## Conventional Commit Format (enforced by prompt)

```
type(scope): subject
```

- **type**: feat, fix, docs, style, refactor, perf, test, build, ci, chore, revert
- **scope**: optional, only when diff makes it obvious
- **subject**: lowercase, imperative, no trailing period, ≤72 chars
- **No body, no quotes, no markdown**

---

## Key Decisions from Discussion

| Topic | Decision |
|---|---|
| Platform | Windows x64 only |
| Config storage | Plain JSON at `%APPDATA%\xommit\config.json` |
| Binary location | `%LOCALAPPDATA%\xommit\bin\xommit.exe` + `version.txt` |
| Provider order | Google first (Recommended), Vercel second |
| Model picker | 10 rows, type-to-filter, arrows, PgUp/PgDn, Home/End |
| Retry policy | One retry on 5xx/network; never on 401/403/429 |
| Output tokens | 512 or model max if lower |
| Empty index | Run `git commit` no args, show git output |
| All ignored | Print message, exit 0 |
| Code signing | Skipped, document SmartScreen in install notes |
| Version start | 0.1.0 |
| Repo | `abhraneeldhar7/xommit` |
| npm scope | `@abhraneeldhar7/xommit` |
| Logo | From `salvage/terminal_ui.h`, ≥80 cols only |
| Old code reuse | Logo, password input, picker/spinner concepts only |

---

## Non-Goals

- Linux/macOS builds
- Code signing
- Environment variable API key fallback
- Complex git operations (branch, push, rebase)
- Tests (deferred)
- Marketing/landing pages (webapp is separate)