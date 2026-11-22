# xommit

Generate conventional commit messages from staged changes, then commit.

xommit reads the staged diff, asks your AI provider for three commit message options, and runs `git commit -m` with the one you pick. Only staged files are sent and only staged files are committed. Windows x64 only.

## Install

### npm

```sh
npm install -g @abhraneeldhar7/xommit
```

The npm package ships a small Node shim. The first run downloads the matching `xommit.exe` release into `%LOCALAPPDATA%\xommit\bin`.

### PowerShell

```powershell
irm https://raw.githubusercontent.com/abhraneeldhar7/xommit/master/cli/scripts/install.ps1 | iex
```

### Git Bash

```sh
curl -fsSL https://raw.githubusercontent.com/abhraneeldhar7/xommit/master/cli/scripts/install.sh | bash
```

## Use

```sh
git add .
xommit
xommit "fix login redirect"
```

On first use:

```sh
xommit --connect
```

That flow asks for a provider, an API key, and a model. Google AI Studio and Vercel AI Gateway are supported.

Other flags:

```sh
xommit --model     # change the model for the current provider
xommit --reset     # delete the saved config
xommit --update    # download the latest release
xommit --version
xommit --help
```

## Config

The config lives at `%APPDATA%\xommit\config.json` and holds the provider, API key, and model id. `--connect` overwrites it. `--reset` deletes it.

Optional ignore rules can be added to `.xommitignore` at the repo root. Ignored files are left out of the AI context, but they are still committed with the rest of the index.

## More documentation

The full documentation lives in the repository `docs/` folder:

- [docs/README.md](../docs/README.md) is the index.
- [docs/getting-started.md](../docs/getting-started.md) covers install and the first run.
- [docs/commands.md](../docs/commands.md) lists every command and exit code.
- [docs/commit-flow.md](../docs/commit-flow.md) explains what happens between `xommit` and `git commit`.
- [docs/providers.md](../docs/providers.md) covers the provider APIs and error handling.

`cli/TESTING.md` explains the test layers, and `notes/` keeps the early design notes for install routes, publishing, providers, the prompt, and the architecture.
