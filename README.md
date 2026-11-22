# xommit

xommit turns staged git changes into conventional commit messages. It reads the staged diff, sends it to an AI provider, shows you three commit message options, and runs `git commit -m` with the one you pick.

I built xommit because I wanted consistent commit messages without leaving the terminal or opening a browser. It only reads staged files, only commits staged files, and never touches your index.

- Windows x64
- Single static binary, no runtime dependencies
- Two providers: Google AI Studio and Vercel AI Gateway
- Free to use with a free provider tier

## Table of contents

- [Install](#install)
- [Quick start](#quick-start)
- [Usage](#usage)
- [How it works](#how-it-works)
- [Configuration](#configuration)
- [Ignoring files](#ignoring-files)
- [Message format](#message-format)
- [Providers](#providers)
- [Updating](#updating)
- [Build from source](#build-from-source)
- [Testing](#testing)
- [Project layout](#project-layout)
- [Documentation](#documentation)
- [License](#license)

## Install

Pick whichever route fits how you work. All three install the same binary to `%LOCALAPPDATA%\xommit\bin`.

### npm

```sh
npm install -g @abhraneeldhar7/xommit
```

The npm package ships a small Node shim instead of the binary. On first run the shim downloads the matching `xommit.exe` release into `%LOCALAPPDATA%\xommit\bin` and runs it. The shim checks `version.txt` first, so it never replaces a newer binary you installed with `xommit --update`.

### PowerShell

```powershell
irm https://raw.githubusercontent.com/abhraneeldhar7/xommit/master/cli/scripts/install.ps1 | iex
```

To pin a version, download the script and pass it explicitly:

```powershell
.\install.ps1 -Version <version>
```

The script adds the install directory to your user PATH. Open a new terminal after it runs.

### Git Bash

```sh
curl -fsSL https://raw.githubusercontent.com/abhraneeldhar7/xommit/master/cli/scripts/install.sh | bash
```

The script appends the install directory to `~/.bashrc`.

### Requirements

- Windows x64
- Git available on PATH
- Node.js 18 or newer, only if you use the npm route
- An API key from one of the supported providers

## Quick start

```sh
# 1. Save your provider, API key, and model
xommit --connect

# 2. Stage what you want to commit
git add .

# 3. Generate and commit
xommit
```

Add a hint when you want to steer the result:

```sh
xommit "fix login redirect"
```

The hint is guidance, not the answer. I instruct the model to describe what actually changed in the diff, so if your hint and the diff disagree, the diff wins.

## Usage

```
xommit [hint...]
xommit --connect
xommit --model
xommit --reset
xommit --update
xommit --version
xommit --help
```

| Command | What it does |
| --- | --- |
| `xommit` | Start a commit with an empty hint |
| `xommit "text here"` | Start a commit and pass the words as a hint |
| `xommit --connect` | Choose a provider, save an API key, pick and test a model |
| `xommit --model` | Change the model for the provider you already saved |
| `xommit --reset` | Delete the saved config |
| `xommit --update` | Download the latest release |
| `xommit --version`, `-v` | Print the version |
| `xommit --help`, `-h` | Print usage |

Any argument that starts with `--` and is not recognized prints a "Command not recognized" message and exits with code 1. Everything else is treated as hint words and joined with spaces.

The full command reference, including every exit code and every failure message, is in [docs/commands.md](docs/commands.md).

## How it works

1. **Read the staged diff.** xommit runs `git diff --cached` with color, external diff, and text conversion disabled. Only staged content is read.
2. **Split it into files.** Diff headers such as `index`, `---`, `+++`, mode lines, and rename lines are dropped. Binary files become a `(binary file)` marker.
3. **Apply ignore rules.** Built-in patterns plus `.xommitignore` remove noisy files from what the model sees.
4. **Load the config.** `%APPDATA%\xommit\config.json` supplies the provider, API key, and model.
5. **Resolve the model.** The provider model list is fetched and the saved model id is looked up, so the context window and output limits are accurate.
6. **Budget the context.** Large diffs are trimmed with a water-filling algorithm so the request fits inside the model context window.
7. **Call the provider.** The system prompt, your hint, and the trimmed diff are sent. Network errors and 5xx responses are retried once.
8. **Parse the response.** Up to three messages are extracted, validated, cleaned, and deduplicated.
9. **Pick one.** An interactive picker shows the options. In a non-interactive shell it falls back to a numbered list.
10. **Commit.** xommit runs `git commit -m "<chosen>"` with the message quoted correctly for the Windows command line.

If nothing is staged, xommit says so and exits without calling the provider. If every staged file is ignored by the ignore rules, it says so and exits without calling the provider. Nothing is added to the index in any case.

The full flow with a diagram is in [docs/commit-flow.md](docs/commit-flow.md).

## Configuration

The config file lives at `%APPDATA%\xommit\config.json`:

```json
{
  "provider": "google",
  "api_key": "your-api-key",
  "model_id": "gemini-2.5-flash-lite"
}
```

- `xommit --connect` writes the file.
- `xommit --reset` deletes it.
- There is no environment variable fallback. The API key lives only in this file.
- If the file exists but is corrupt or incomplete, xommit reports the problem and tells you to run `xommit --reset` instead of pretending the config is missing.

Details are in [docs/configuration.md](docs/configuration.md).

## Ignoring files

You can exclude files from what the model sees with a `.xommitignore` file at the repository root:

```
# comments are allowed
dist/
*.lock
!src/important.lock
```

- Blank lines and lines starting with `#` are skipped.
- `!pattern` re-includes a path.
- `dir/` matches a directory at any depth.
- A pattern containing `/` is globbed against the whole path.
- Any other pattern matches the basename or any path segment.
- `*` and `?` are supported.

xommit already ignores common lockfiles, build output, dependency folders, binary assets, and minified files by default.

Ignore rules only affect the AI context. Ignored files are still committed with the rest of the index.

## Message format

Every message follows Conventional Commits:

```
type(scope): subject
```

- **type**: `feat`, `fix`, `docs`, `style`, `refactor`, `perf`, `test`, `build`, `ci`, `chore`, `revert`
- **scope**: optional, and only a short area name such as `auth`, `api`, `cli`, `ui`, or `parser`. Never a file name or directory.
- **subject**: lowercase, imperative, no trailing period, 72 characters maximum.

The parser rejects anything that does not match this shape before you ever see it. It also rejects double-prefixed messages like `fix: fix(app): update parser`, and messages where the subject starts with another `type(scope):` prefix.

Details are in [docs/prompt.md](docs/prompt.md).

## Providers

| Provider | Endpoint | Notes |
| --- | --- | --- |
| Google AI Studio | `generativelanguage.googleapis.com/v1beta` | Recommended. The free tier covers normal use. |
| Vercel AI Gateway | `ai-gateway.vercel.sh/v1` | Needs a Vercel account and gateway key. |

Both providers expose their model list, and xommit ranks it. Google models are ordered `flash-lite`, `flash`, `pro`, then the rest, with preview and non-text models pushed down. Vercel models are ordered free first, then cheapest, then largest context. The first model in the list is marked as recommended.

Each request asks for at most 512 output tokens and uses a temperature of 0.2.

Provider requests and error handling are covered in [docs/providers.md](docs/providers.md).

## Updating

```sh
xommit --update
```

xommit asks GitHub for the latest release tag. If it is newer than the running version, it downloads the release asset, checks the file size, renames the current binary to `xommit.old.exe`, moves the new binary into place, and writes `version.txt`. The old binary is deleted on the next start.

More detail is in [docs/update-flow.md](docs/update-flow.md).

## Build from source

```sh
cd cli
npm install
npm run build
```

The output is `cli/bin/xommit.exe`.

You need MinGW-w64 UCRT64 GCC on PATH (`g++` must resolve) and Node.js 18 or newer. The build script reads the version from `cli/package.json` and links statically so the binary has no runtime dependencies.

## Testing

```sh
cd cli
npm test          # unit tests and smoke tests, no API calls
npm run test:live # also runs live provider tests
```

The suite has three layers:

- **Unit tests** are a plain C++ executable built from `cli/tests/test_logic.cpp`. They cover diff parsing, ignore rules, context budgeting, prompt building, and commit message validation.
- **Smoke tests** are a dependency-free Node runner in `cli/tests/run.js`. They build the real binary and exercise it against throwaway git repositories.
- **Live tests** run the same runner with `--live`. They use your real config and spend provider credits, so they are rate limited and are not meant for CI.

I keep the live tests out of the default run so that `npm test` stays free and deterministic. See [docs/testing.md](docs/testing.md) and [cli/TESTING.md](cli/TESTING.md).

## Project layout

```
xommit/
├── .github/workflows/release.yml   # Release pipeline
├── AGENTS.md                       # Instructions for coding agents
├── README.md                       # This file
├── cli/
│   ├── package.json                # Version source of truth
│   ├── README.md                   # npm package landing page
│   ├── include/xommit/             # Public headers
│   ├── src/                        # Implementation, one file per area
│   ├── scripts/                    # Build, npm shim, installers, release
│   ├── tests/                      # Unit tests and the test runner
│   ├── TESTING.md                  # Test documentation
│   └── bin/                        # Build output, gitignored
├── docs/                           # Full documentation
├── notes/                          # Early design notes
├── salvage/                        # Salvaged logo and UI experiments
└── webapp/                         # Separate web project, unrelated to the CLI
```

## Documentation

Everything lives in [docs/](docs/README.md):

| Document | Contents |
| --- | --- |
| [docs/getting-started.md](docs/getting-started.md) | Install, first run, connect a provider |
| [docs/commands.md](docs/commands.md) | Every command, flag, exit code, and error message |
| [docs/commit-flow.md](docs/commit-flow.md) | The main commit flow and its scenarios |
| [docs/setup-flow.md](docs/setup-flow.md) | `--connect` and `--model` flows |
| [docs/update-flow.md](docs/update-flow.md) | Self update flow |
| [docs/configuration.md](docs/configuration.md) | Config file and ignore rules |
| [docs/providers.md](docs/providers.md) | Provider APIs, ranking, retries, errors |
| [docs/prompt.md](docs/prompt.md) | Prompt, response parsing, validation |
| [docs/context.md](docs/context.md) | Context budgeting and truncation |
| [docs/terminal.md](docs/terminal.md) | TTY and non-TTY behavior, picker controls |
| [docs/architecture.md](docs/architecture.md) | Module map and data flow |
| [docs/testing.md](docs/testing.md) | Test layers and how to run them |
| [docs/release.md](docs/release.md) | Build, tag, and publish pipeline |

## License

MIT
