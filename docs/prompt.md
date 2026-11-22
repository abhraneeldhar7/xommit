# The prompt and the response

This covers what xommit asks the model for and how it decides whether the answer is usable.

## The system prompt

Sent as `systemInstruction` on Google and as the `system` message on Vercel.

```text
You write git commit messages in Conventional Commits format.

Return exactly three different options as JSON and nothing else:
{"commits":["<option 1>","<option 2>","<option 3>"]}

Rules:
- Each option is a single line: type(scope): subject
- type is one of: feat, fix, docs, style, refactor, perf, test, build, ci, chore, revert
- Choose the type that matches the main intent:
  feat = new user-facing behavior, fix = corrects broken behavior,
  docs = documentation only, refactor = code change with no behavior change,
  test = tests only, build = build system or dependencies, ci = CI config,
  perf = performance, style = formatting only, chore = maintenance,
  revert = reverts a previous commit
- Each option must summarize the whole diff, not one file or one hunk. When several files change, describe the shared intent in every option. Never write an option that covers only one file.
- All three options must cover the whole diff; vary only wording or emphasis.
- When several files change, use a subject like "add subtract function and document usage", not one that mentions only the code or only the docs.
- scope is optional. Use a short module or area name such as auth, api, cli, ui, or parser. Never use a file name, directory, path, or extension as the scope. Bad scopes: readme, src, lib, js.
- README, comments, and documentation changes use docs, even when correcting a typo.
- If any code file changes, do not use docs. A new function plus README changes is feat, not docs.
- Use exactly one type and one optional scope. Do not put another type(scope): prefix inside the subject.
- Do not use feat unless the diff adds new behavior. Do not use fix unless the diff corrects broken behavior. Do not use refactor unless behavior is unchanged.
- Renaming variables, extracting functions, or restructuring code without changing behavior is refactor, even when many lines change.
- subject is lowercase, imperative, and has no trailing period
- keep each option at or under 72 characters
- describe the actual change in the diff, not the user's wording
- if the user hint and the diff disagree, trust the diff
- all three options must be valid and distinct; vary wording or emphasis, not correctness
- no body, no quotes, no markdown, no explanations
- if the diff is truncated, use the visible changes only
```

The JSON shape is the contract. Anything else is treated as a malformed response and falls back to the line based parser.

## The user prompt

```text
User hint: <your hint, or (none)>

<diff context>
```

The diff context is a list of file blocks:

```text
<src/parser.cpp>
@@ -1,4 +1,6 @@
-old line
+new line
</src/parser.cpp>

<README.md>
@@ -1 +1 @@
-old
+new
</README.md>
```

Binary files appear as `(binary file)`. A file with no hunks appears as `(no content changes)`.

When the context had to be trimmed, the whole context is prefixed with:

```text
Note: some diffs were truncated to fit the context window.
```

And a trimmed file ends with:

```text
... truncated
```

## Parsing the response

The parser runs in two stages.

### Stage one: JSON

The response is scanned for the first `{` or `[` and the last `}` or `]`. That slice is parsed as JSON.

Accepted shapes:

```json
{"commits": ["fix(cli): quote commit message", "fix(cli): quote the commit message"]}
```

```json
["fix(cli): quote commit message", "fix(cli): quote the commit message"]
```

Only string items in the `commits` array, or string items in the top level array, are considered. Any other item is skipped.

If the JSON parse fails or produces no valid strings, stage two runs.

### Stage two: lines

The response is read line by line.

- Blank lines are skipped.
- Lines starting with ` ``` `, `{`, `[`, or `"` are skipped.
- A leading `-`, `*`, or `+` is removed.
- A leading number followed by `.` or `)` is removed, so `1. fix(cli): ...` becomes `fix(cli): ...`.

## Validation

A candidate message is kept only if it passes every check.

### Format check

```text
type(scope): subject
```

| Check | Rule |
| --- | --- |
| Colon | There must be a colon |
| Type | Must be one of the eleven known types, case insensitive |
| Breaking marker | A trailing `!` on the type is allowed, for example `feat(api)!: ...` |
| Scope | If present, it must be a complete `(...)` group |
| Subject | Must not be empty after trimming |

### Double prefix check

The subject must not start with another type prefix. These are rejected:

```text
fix: fix(cli): quote commit message
fix(cli): feat: add parser
chore: fix(auth): validate tokens
```

The check catches `type:` and `type(` at the start of the subject, and the same patterns after a space.

### Cleaning

Every surviving candidate goes through `sanitize_message`:

1. Carriage returns and newlines become spaces.
2. Runs of whitespace collapse into one space.
3. A surrounding pair of double quotes is removed.
4. Trailing periods are removed.
5. If the result is longer than 72 characters, it is cut at the last space before the limit. If there is no usable space, it is cut at 72. The cut never lands in the middle of a UTF-8 sequence.

### Deduplication

Identical messages are dropped. At most three are kept.

### Empty result

If nothing survives, xommit prints:

```text
error: could not read commit messages from the model response
```

and exits 1 without committing.

## Why the type rules are so specific

The rules exist because of real failure cases I hit while testing:

- **Wrong type.** The model was using `docs` for a change that also touched code. The rules now say: if any code file changes, it is not `docs`.
- **Single file options.** The model returned one option per file. The rules now require every option to summarize the whole diff.
- **Bad scopes.** The model returned `readme`, `src`, or `parser.cpp` as a scope. The rules now require a short area name and forbid file names and paths.
- **Double prefixes.** The model sometimes returned `fix: fix(cli): ...`. That is now rejected by the parser, not just discouraged in the prompt.
- **Truncated diffs.** The model invented changes that were cut out of the context. The rules now say to describe only what is visible.

## Cost of the prompt

The system prompt is roughly 700 tokens. It is subtracted from the context budget along with your hint and a fixed margin. See [context.md](context.md).

Next: [context.md](context.md)
