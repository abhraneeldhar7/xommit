# Prompt

## System prompt

Sent as `systemInstruction` on Google and as the `system` message on Vercel.

```
You write git commit messages in Conventional Commits format.

Return exactly three different options as JSON and nothing else:
{"commits":["<option 1>","<option 2>","<option 3>"]}

Rules:
- Each option is a single line: type(scope): subject
- type is one of: feat, fix, docs, style, refactor, perf, test, build, ci, chore, revert
- scope is optional and only when the diff makes it obvious
- subject is lowercase, imperative, and has no trailing period
- keep each option at or under 72 characters
- describe the actual change in the diff, not the user's wording
- if the user hint and the diff disagree, trust the diff
- no body, no quotes, no markdown, no explanations
- if the diff is truncated, use the visible changes only
```

The JSON shape is the contract. Anything else is treated as a malformed response.

## User prompt

```
User hint: <hint or (none)>

<diff context>
```

The diff context is a list of file blocks:

```
<path/to/file>
@@ ... diff lines ...
</path/to/file>

<other/file>
...
</other/file>
```

Only staged, non ignored files appear. Binary files appear as `(binary file)`. When the budget cuts content, the block ends with `... truncated` and the whole context is prefixed with a note that some diffs were truncated.

## Parsing

The response is scanned for the first `{` or `[` and the last `}` or `]`. That slice is parsed as JSON.

Accepted shapes:

- `{"commits": ["a", "b", "c"]}`
- `["a", "b", "c"]`

If JSON parsing fails or yields no strings, the response is read line by line:

- blank lines are skipped
- code fences and lines starting with `{` are skipped
- a leading `-`, `*`, or `+` is removed
- a leading `1.` or `1)` is removed

## Cleaning

Every option goes through:

1. Newlines and carriage returns become spaces.
2. Runs of whitespace collapse into one space.
3. A surrounding pair of double quotes is removed.
4. A trailing period is removed.
5. Anything longer than 72 characters is cut at the last space before the limit, or at 72 when there is no usable space.

Duplicates are dropped. At most three options are kept.

If no option survives, the CLI prints an error and ends without committing.
