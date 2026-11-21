# Architecture

## Layout

```
cli/
  include/xommit/   headers
  src/              implementation, one .cpp per area
  scripts/          build, npm shim, installers, release
  package.json      version source
```

`src/main.cpp` sets UTF-8 console output, enables ANSI escape processing, calls `run_cli`, and turns any escaping exception into `error: <message>` with exit code 1.

## Arguments

`run_cli` in `src/cli.cpp`:

- no arguments: commit flow with an empty hint
- `--help`, `-h`: usage
- `--version`, `-v`: version
- `--connect`: provider, key, and model setup
- `--reset`: delete the config
- `--update`: self update
- anything else: joined with spaces and used as the hint

Only the first known flag wins. Unknown flags are hint words.

## Commit flow

1. Print the logo when the terminal is at least 80 columns wide.
2. Resolve the repo root with `git rev-parse --show-toplevel`.
3. Read the staged diff with:

   ```
   git -c core.quotepath=false diff --cached --no-color --no-ext-diff --no-textconv -M
   ```

4. If the diff is empty, run `git commit` with no arguments, let git print its own message, and return its exit code.
5. Split the diff into files. Headers such as `index`, `---`, `+++`, mode lines, and rename lines are dropped. Binary entries become `(binary file)`.
6. Drop files matched by the ignore rules.
7. If nothing is left, print `only ignored files are staged, nothing to send to the AI` and exit 0. The index is untouched.
8. Load the config. A missing or broken config ends the flow with a pointer to `--connect`.
9. Budget the context, build the prompt, and call the provider.
10. Parse up to three messages.
11. Show the picker and run `git commit -m <choice>`.

Only staged content is read and only staged content is committed. Nothing is added to the index by xommit.

## Ignore rules

`src/ignore.cpp` starts with a built in list of lock files, build output, dependencies, and binary assets. It then appends lines from `.xommitignore` at the repo root.

Rules:

- blank lines and lines starting with `#` are skipped
- `!pattern` re-includes
- `dir/` matches a directory at any depth
- a pattern containing `/` is globbed against the whole path
- any other pattern matches the basename or any path segment
- `*` and `?` are supported

Ignore rules only affect what the model sees. They never change what `git commit` includes.

## Context budget

`context_budget_chars` in `src/diff.cpp`:

1. Start with the model context window, defaulting to 32000 when unknown.
2. Reserve output tokens: 512, or the model maximum when it is lower.
3. Subtract the system prompt, the hint, and a small fixed margin.
4. Keep 90 percent of what remains.
5. Convert tokens to characters at 4 characters per token.

Files are then fitted with a water filling loop. Each round divides the remaining budget by the number of pending files. Files that fit entirely are accepted and their real size is subtracted. The rest go to the next round with a bigger share. When no file fits, every remaining file gets an equal slice and is cut at a line boundary.

If the final context is empty, the flow stops with an error instead of sending a useless prompt.

## Output reserve

The same 512 token value is sent as `maxOutputTokens` and `max_tokens`. A lower model maximum wins. This keeps the request within the model limit and matches the budget calculation.

## Terminal behavior

- The logo is printed only when stdout is a TTY and the window is at least 80 columns.
- Colors and cursor movement are skipped when stdout is not a TTY.
- The spinner writes to a single line and clears itself. When not a TTY it prints the label once.
- The password prompt uses `_getch`, echoes `*`, supports backspace, Esc to cancel, and Ctrl+C to exit.
- The picker is a 10 row viewport with type to filter, arrow keys, PageUp, PageDown, Home, End, Enter, and Esc.
- When stdin or stdout is not a TTY, the picker falls back to a numbered list read with `getline`. An invalid number cancels.

Cancelling any picker returns exit code 1 and commits nothing.

## Git calls

Reads use `CreateProcessW` with redirected pipes and two reader threads, so large diffs cannot deadlock. `CREATE_NO_WINDOW` keeps a console from flashing.

Writes use `_wspawnvp` with inherited standard handles, so `git commit` shows its normal output, hooks, and editor behavior.

The commit message is converted to UTF-16 before being passed to `git commit -m`, so non ASCII text survives.

## Update

`--update` asks the GitHub API for the latest release tag, compares it with the compiled in version, downloads `xommit.exe`, and swaps it in through a rename because the running file cannot be overwritten. The previous binary is kept as `xommit.old.exe` until the next start.
