# Configuration

xommit has two configuration surfaces: the config file for credentials and the model, and `.xommitignore` for controlling what the model sees.

## The config file

### Location

```text
%APPDATA%\xommit\config.json
```

`APPDATA` is used when it is set. If it is not, xommit falls back to `%USERPROFILE%\AppData\Roaming`.

### Format

```json
{
  "provider": "google",
  "api_key": "your-api-key",
  "model_id": "gemini-2.5-flash-lite"
}
```

| Field | Meaning | Valid values |
| --- | --- | --- |
| `provider` | Which API to call | `google`, `vercel` |
| `api_key` | The secret for that provider | Any string the provider accepts |
| `model_id` | Which model to use | A model id from the provider's model list |

The file is written by `xommit --connect` and by `xommit --model`. It is deleted by `xommit --reset`.

### What is not configurable

- There is no environment variable fallback for the API key. I keep the key in exactly one place so it is easy to find and easy to wipe.
- There is no config option for temperature, output tokens, or the context budget. Those follow the model's own limits and the system prompt.
- There is no per-repository config. The config is global for your user account.

### Reading and validation

`load_config` reports these states separately:

| State | Message |
| --- | --- |
| Missing | The connect flow runs instead of an error |
| Empty file | `no saved credentials` |
| Invalid JSON | `config file is corrupt, run xommit --reset` |
| Missing `provider`, `api_key`, or `model_id` | `config file is incomplete, run xommit --reset` |

The corrupt and incomplete cases are errors on purpose. Treating them as "no config" would silently start the connect flow and overwrite a key you may still want.

## The ignore file

### Location

```text
<repo root>\.xommitignore
```

The repo root comes from `git rev-parse --show-toplevel`. The file is optional.

### What it controls

Only the AI context. Ignored files are still committed with the rest of the index, because xommit runs `git commit` on the index as it was and never touches it.

### Built-in patterns

These are always applied, before your own rules:

```text
package-lock.json
pnpm-lock.yaml
yarn.lock
Cargo.lock
go.sum
poetry.lock
composer.lock
Gemfile.lock
*.min.js
*.min.css
*.map
dist/
build/
out/
node_modules/
vendor/
*.png
*.jpg
*.jpeg
*.gif
*.ico
*.webp
*.pdf
*.zip
*.gz
*.7z
*.exe
*.dll
*.so
*.dylib
*.woff
*.woff2
*.ttf
```

### Rule syntax

| Rule | Meaning |
| --- | --- |
| Empty line | Skipped |
| `# comment` | Skipped |
| `!pattern` | Re-includes a path that an earlier rule ignored |
| `dir/` | Matches that directory at any depth |
| `pattern/with/slash` | Globbed against the whole path |
| `plain` | Matched against the basename or any single path segment |
| `*` | Matches any run of characters |
| `?` | Matches exactly one character |

Rules are evaluated in order. The last matching rule wins, so a negation at the end of the file can bring a file back in.

### Examples

Keep everything but the generated folder:

```
dist/
```

Keep a lockfile that a rule above ignored:

```
*.lock
!pnpm-lock.yaml
```

Ignore one directory only:

```
packages/api/generated/
```

Ignore a file by name anywhere:

```
.env
```

### Edge cases

- A UTF-8 BOM at the start of the file is skipped.
- Paths are compared with forward slashes, so a Windows path is normalized first.
- The built-in patterns come first, so a `!pattern` in `.xommitignore` can re-include a built-in match.

## Environment variables

| Variable | Purpose |
| --- | --- |
| `APPDATA` | The config directory |
| `LOCALAPPDATA` | The binary directory |
| `USERPROFILE` | Fallback for both of the above |

Next: [providers.md](providers.md)
