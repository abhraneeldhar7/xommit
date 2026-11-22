# Terminal behavior

xommit is a Windows console program, so it has to behave well in a real terminal, in a pipe, in CI, and in a script. This page describes how it detects each case and what changes.

## TTY detection

Two checks are used:

| Check | Function |
| --- | --- |
| Is stdin a terminal | `_isatty(_fileno(stdin))` |
| Is stdout a terminal | `_isatty(_fileno(stdout))` |

The console is switched to UTF-8 at startup, and virtual terminal processing is enabled when the output handle is a real console.

## The logo

| Condition | What prints |
| --- | --- |
| stdout is a TTY and the window is 80 columns or wider | The six line ASCII banner, colored |
| stdout is a TTY and the window is narrower | A single `xommit` line, colored |
| stdout is not a TTY | A single plain `xommit` line, no escape codes |

The narrow and non-TTY versions contain no ANSI sequences at all. This matters for logs and for tests that assert on the output.

## Colors

Every color helper checks `stdout_is_tty()` and returns an empty string when it is false. So the same code path produces clean text in a pipe.

| Style | Escape |
| --- | --- |
| Bold | `\033[1m` |
| Dim | `\033[90m` |
| Green | `\033[1;32m` |
| Cyan | `\033[1;36m` |
| Red | `\033[1;31m` |
| Reset | `\033[0m` |
| Link | OSC 8 hyperlink |

## The spinner

| Condition | Behavior |
| --- | --- |
| stdout is a TTY | A four frame animation (`o`, `0`, `O`, `0`) every 120 ms on one line, cleared when the work finishes |
| stdout is not a TTY | The label is printed once as a normal line |

The spinner is used while fetching models, while calling the provider, and while downloading a release.

## The password prompt

| Condition | Behavior |
| --- | --- |
| stdin is a TTY | `_getch` loop, `*` for each character, backspace erases, Enter submits, Esc cancels, Ctrl+C exits with 130 |
| stdin is not a TTY | A normal line read, echoed by the terminal |

Ctrl+C during the key prompt exits the process immediately with code 130.

## The picker

`select_option` uses the interactive picker only when both stdin and stdout are terminals. Otherwise it uses the numbered fallback.

### Interactive picker

```text
Choose a commit message:
Type to search: fix
  > fix(cli): quote commit message arguments
    fix(cli): quote the commit message arguments
    fix(cli): quote commit message for windows
  up/down navigate, type to filter, enter select, esc cancel
  (1/3)
```

| Key | Action |
| --- | --- |
| Up, Down | Move the selection by one |
| PageUp, PageDown | Move by ten |
| Home | Jump to the first match |
| End | Jump to the last match |
| Any printable character | Append to the filter and reset the selection |
| Backspace | Remove one character from the filter |
| Enter | Select the highlighted option |
| Esc | Cancel |
| Ctrl+C | Cancel |

The viewport shows ten rows at a time and scrolls to keep the selection visible. Long labels are truncated to the terminal width with `...`.

Filtering is a case-insensitive substring match on the label.

The cursor is hidden while the picker is open and restored when it closes.

### Numbered fallback

```text
Choose a commit message:
  1. fix(cli): quote commit message arguments
  2. fix(cli): quote the commit message arguments
  3. fix(cli): quote commit message for windows
Select [1-3]:
```

The input is read with `getline`. A number outside the range cancels. A non-numeric value also cancels, because it parses as zero.

This is what runs in CI, in pipes, and in the smoke tests.

## Cancelling

Cancelling any picker prints `cancelled` and exits 1. Nothing is committed and nothing is saved.

## Line clearing

`clear_lines` moves the cursor up and erases each line. It does nothing when stdout is not a TTY, so no escape codes leak into logs.

Next: [architecture.md](architecture.md)
