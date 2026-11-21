# xommit

Generate conventional commit messages for staged changes, then commit.

xommit reads the staged diff, asks your AI provider for three commit message options, and runs `git commit -m` with the one you pick. Only staged files are sent and committed. Windows x64 only.

## Install

### npm

```sh
npm install -g @abhraneeldhar7/xommit
```

The npm package ships a small Node shim. The first run downloads the pinned `xommit.exe` release into `%LOCALAPPDATA%\xommit\bin`.

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

Other flags:

```sh
xommit --reset     # delete the saved config
xommit --update    # download the latest release
xommit --version
xommit --help
```

## Config

The config lives at `%APPDATA%\xommit\config.json` and holds the provider, API key, and model. `--connect` overwrites it. `--reset` deletes it.

Optional ignore rules can be added to `.xommitignore` at the repo root. Ignored files are left out of the AI context, but they are still committed with the rest of the index.

## Notes

More detail lives in the repository `notes/` folder: install routes, publishing, providers, the prompt, and the architecture.
