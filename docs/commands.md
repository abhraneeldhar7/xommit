# Command reference

Every command, every flag, every exit code.

## Argument parsing

Arguments are read from left to right. The first recognized command wins and the rest are ignored.

| Argument | Result |
| --- | --- |
| `--help`, `-h` | Print usage and exit 0 |
| `--version`, `-v` | Print `xommit <version>` and exit 0 |
| `--connect` | Run the connect flow |
| `--model` | Run the model selection flow |
| `--reset` | Delete the saved config |
| `--update` | Run the self update flow |
| Anything starting with `--` that is not listed above | Print "Command not recognized, use --help" and exit 1 |
| Anything else | Collected as hint words and joined with spaces |

Hint words are joined with a single space, so these are equivalent:

```sh
xommit "fix login redirect"
xommit fix login redirect
```

If there are no arguments at all, the hint is empty and the commit flow still runs.

## Commands

### `xommit`

Runs the commit flow. See [commit-flow.md](commit-flow.md).

Exit codes:

| Code | When |
| --- | --- |
| 0 | The commit succeeded |
| 0 | Nothing was staged |
| 0 | Every staged file was ignored by the ignore rules |
| 1 | The provider call failed |
| 1 | The response contained no usable commit message |
| 1 | You cancelled the picker |
| 1 | The config file is corrupt or incomplete |
| 1 | The model could not be resolved |

xommit exits with git's own exit code when the commit itself fails, because git's output is shown as-is.

### `xommit "<hint>"`

Same as `xommit`, with the given text sent to the model as guidance.

One special case: if there is no saved config, xommit runs the connect flow first. With a hint, the commit continues automatically after the config is saved. Without a hint, the connect flow finishes and xommit stops.

### `xommit --connect`

Interactive setup. See [setup-flow.md](setup-flow.md).

1. Choose a provider.
2. Paste an API key.
3. Choose a model.
4. xommit sends a test request to confirm the model works.
5. On success the config is saved.

| Code | When |
| --- | --- |
| 0 | The config was saved |
| 1 | You cancelled the provider or model picker |
| 1 | The API key was empty |
| 1 | The model list could not be fetched |
| 1 | The provider request failed |

Running `--connect` overwrites the entire existing config. The provider, the key, and the model are all replaced.

### `xommit --model`

Changes only the model, keeping the provider and the API key.

1. The saved config is loaded.
2. The model list is fetched with the saved key.
3. You pick a model.
4. xommit sends a test request.
5. On success only `model_id` is rewritten.

| Code | When |
| --- | --- |
| 0 | The model was saved |
| 1 | There is no config yet, the connect flow runs instead |
| 1 | The config file is corrupt or incomplete |
| 1 | The model list could not be fetched |
| 1 | The key was rejected, the connect flow runs instead |
| 1 | You cancelled the model picker |

### `xommit --reset`

Deletes `%APPDATA%\xommit\config.json`. It prints a success message whether or not the file existed. Exit code is always 0.

### `xommit --update`

See [update-flow.md](update-flow.md).

| Code | When |
| --- | --- |
| 0 | The binary was updated, or you were already on the latest version |
| 1 | GitHub could not be reached, or the latest release could not be read |
| 1 | The downloaded file looked invalid |
| 1 | The running binary could not be replaced |

### `xommit --version`

Prints `xommit <version>` and exits 0.

### `xommit --help`

Prints the usage screen and exits 0.

## Exit codes in full

| Code | Meaning |
| --- | --- |
| 0 | Success, including "nothing to do" outcomes |
| 1 | Any handled failure |
| 130 | You pressed Ctrl+C while typing the API key |
| Git's code | When `git commit` itself fails |

## Environment variables

xommit reads two environment variables for its own paths.

| Variable | Used for | Fallback |
| --- | --- | --- |
| `APPDATA` | The config directory | `%USERPROFILE%\AppData\Roaming` |
| `LOCALAPPDATA` | The binary directory | `%USERPROFILE%\AppData\Local` |

There is no environment variable for the API key on purpose. The key is only ever read from the config file.

## Files xommit reads or writes

| Path | Purpose |
| --- | --- |
| `%APPDATA%\xommit\config.json` | Provider, API key, model id |
| `%LOCALAPPDATA%\xommit\bin\xommit.exe` | The installed binary |
| `%LOCALAPPDATA%\xommit\bin\version.txt` | The installed version stamp |
| `%LOCALAPPDATA%\xommit\bin\xommit.old.exe` | The previous binary, deleted on next start |
| `<repo root>\.xommitignore` | Extra ignore rules |

Next: [commit-flow.md](commit-flow.md)
