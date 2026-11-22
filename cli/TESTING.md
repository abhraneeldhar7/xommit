# Testing

xommit has three test layers. Each layer answers a different question.

| Layer | Question | Network | Speed |
|---|---|---|---|
| Unit | Do the pure functions behave correctly? | no | seconds |
| Smoke | Does the built exe behave correctly as a process? | no | seconds |
| Live | Does the real provider return usable commit messages? | yes | 10-30s |

No test framework is required. The runner is plain Node, and the unit tests are a small C++ executable.

## Run

From `cli/`:

```sh
npm test
```

This builds `bin/xommit.exe`, compiles and runs the unit tests, then runs the CLI smoke tests.

```sh
npm run test:live
```

This runs everything above and then calls the provider configured in `%APPDATA%\xommit\config.json`. It spends API credits and needs a valid config. Do not run it in CI.

Requirements:

- MinGW `g++` on `PATH` (or set `CXX`)
- `git` on `PATH`
- Node 18 or newer

## Unit tests

File: `tests/test_logic.cpp`

The runner compiles every file in `src/` except `main.cpp` together with the test file. That also catches compile errors in the real sources.

Covered:

- `sanitize_message`: whitespace, quotes, trailing periods, 72 byte limit, UTF-8 safety
- `parse_commit_options`: JSON object, JSON array, fenced JSON, prose fallback, bullets, duplicates, malformed entries, double type prefixes
- `parse_diff`: modified, added, deleted, renamed, binary, spaces in paths
- `build_diff_context`: empty input, truncation, budget limits, binary files
- `context_budget_chars` and `estimate_tokens`
- `IgnoreRules`: built-ins, nested paths, `.xommitignore`, negation
- `Json`: parse, dump, escapes, unicode, invalid input
- `Config`: save, load, corrupt file, incomplete file, reset
- `git_commit`: multi-word messages, quotes, backslashes, UTF-8, trailing backslash

The `git_commit` test is a regression test for a real bug: `_wspawnvp` does not quote arguments, so a multi-word message was split into multiple pathspecs. The test commits a message with spaces, quotes, a backslash, and non-ASCII text, then reads the commit back with `git log`.

## CLI smoke tests

File: `tests/run.js`

Each case spawns `bin/xommit.exe` with an isolated `APPDATA` and `LOCALAPPDATA` under a temporary directory. The real config is never touched. Temporary git repos are created under the same directory.

Covered:

- `--version` and `-v`
- `--help` and `-h`, including every documented flag
- unknown flags
- `--reset` with and without a config
- outside a git repository
- empty index
- only ignored files staged
- missing config starts the connect flow
- corrupt config prints the corrupt error instead of silently reconnecting
- incomplete config prints the incomplete error
- `--connect` cancel, invalid choice, and empty key
- `--model` without a config
- no ANSI escape codes when stdout is not a TTY

The smoke tests intentionally use `input: ""` so stdin is a pipe. That exercises the non-interactive paths and the numbered picker fallback.

## Live tests

File: `tests/run.js`, `liveTests()`

The live test:

1. Copies `%APPDATA%\xommit\config.json` into an isolated temp directory.
2. Creates a temp git repo with a small bug fix.
3. Runs the full commit flow with input `1`.
4. Reads the resulting commit message with `git log`.

It asserts the message is conventional, at most 72 characters, single line, and has no trailing period. It prints the chosen message so you can review quality.

The live test does not assert the exact type or wording. Model output is stochastic, so exact assertions would be flaky. Review the printed message when changing the prompt.

## Prompt changes

When changing `SYSTEM_PROMPT` in `src/prompt.cpp`:

1. Run `npm test` to make sure parsing and validation still work.
2. Run `npm run test:live` a few times with different diffs.
3. Check that each option:
   - uses a valid type
   - describes the whole diff, not one file
   - uses a real scope, not a file name
   - stays under 72 characters
   - has no trailing period

A useful manual check is to stage a docs-only change, a refactor, a CI change, and a multi-file feature, then run the CLI with stdin closed (`"" | xommit`) to see the options without committing.

## Isolation

- Unit tests use unique temp directories and remove them at the end.
- Smoke tests use a temporary `APPDATA`, `LOCALAPPDATA`, and git repo.
- Live tests copy the real config into a temporary `APPDATA`; the real file is read only.
- `--update` is not tested automatically because it replaces binaries and hits GitHub. Test it manually in a throwaway `LOCALAPPDATA`.

## CI

The automated layers are CI-friendly on `windows-latest`:

```yaml
- uses: msys2/setup-msys2@v2
  with:
    install: mingw-w64-ucrt-x86_64-gcc
- run: npm test
  working-directory: cli
```

Do not run `npm run test:live` in CI.

## Adding tests

- Pure logic goes in `tests/test_logic.cpp` with `check`, `check_eq`, or `check_eq_int`.
- Process behavior goes in `tests/run.js` with `runCli` and `checkContains`.
- Keep each test independent and use temp directories.
- Add a regression test for every bug that is fixed.
