# Getting started

This guide takes you from nothing to your first xommit commit.

## 1. Install

xommit is a Windows x64 tool. Pick one install route. All three put the same `xommit.exe` in `%LOCALAPPDATA%\xommit\bin`.

### npm

```sh
npm install -g @abhraneeldhar7/xommit
```

I recommend this route if you already use npm. The package contains a Node shim that downloads the binary on first use and keeps it up to date with the installed package version.

### PowerShell

```powershell
irm https://raw.githubusercontent.com/abhraneeldhar7/xommit/master/cli/scripts/install.ps1 | iex
```

The script downloads the latest release asset, verifies it looks like a real binary, moves it into `%LOCALAPPDATA%\xommit\bin`, writes `version.txt`, and adds the directory to your user PATH. Open a new terminal afterwards.

To pin a specific version:

```powershell
.\install.ps1 -Version <version>
```

### Git Bash

```sh
curl -fsSL https://raw.githubusercontent.com/abhraneeldhar7/xommit/master/cli/scripts/install.sh | bash
```

The script does the same thing and appends the directory to `~/.bashrc`.

### Check the install

```sh
xommit --version
```

If that prints a version, you are ready. If your shell says the command is not found, the install directory is not on your PATH yet. Open a new terminal or add `%LOCALAPPDATA%\xommit\bin` yourself.

## 2. Get an API key

xommit needs a key for one of two providers.

| Provider | Where to get a key | Cost |
| --- | --- | --- |
| Google AI Studio | Google AI Studio console, create an API key | Free tier available |
| Vercel AI Gateway | Vercel dashboard, AI Gateway section | Usually paid, some free models |

xommit never stores your key anywhere else. It lives in `%APPDATA%\xommit\config.json` and nowhere else.

## 3. Connect

```sh
xommit --connect
```

You get three steps:

1. **Pick a provider.** Google AI Studio is listed first and marked as recommended.
2. **Paste your API key.** The prompt does not echo what you type.
3. **Pick a model.** The list is sorted so the sensible choices are at the top. Type to filter, arrow keys to move, Enter to choose.

After you choose a model, xommit sends a tiny test request. If it succeeds, the config is saved and you are done. If it fails, the provider error is shown and you go back to the model list without losing your key.

## 4. Make a commit

```sh
git add .
xommit
```

xommit reads the staged diff, asks the provider for three messages, shows them in a picker, and commits the one you select.

With a hint:

```sh
xommit "fix login redirect"
```

The hint is a nudge, not a script. If the diff clearly shows something else, xommit follows the diff.

## 5. Change things later

```sh
xommit --model    # switch model, keep provider and key
xommit --connect  # change provider, key, and model
xommit --reset    # delete the saved config
xommit --update   # download the latest release
```

## 6. Keep noisy files out of the request

By default xommit already skips lockfiles, build output, dependency folders, binary assets, and minified files. For project-specific files, add `.xommitignore` at the repository root:

```
# secrets and local settings
.env
.env.local
*.pem
```

Remember that ignore rules only affect what is sent to the provider. Ignored files are still committed.

## Common problems

**"git is not installed or not on PATH"**

xommit shells out to git. Install git for Windows and make sure `git` runs in your terminal.

**"API key rejected (401). Run xommit --connect."**

The key is wrong, expired, or copied with extra whitespace. Run `xommit --connect` and paste it again.

**"config file is corrupt, run xommit --reset"**

The config file exists but cannot be parsed. Run `xommit --reset`, then `xommit --connect`.

**"No files are staged"**

There is nothing staged. Run `git add` first.

**"only ignored files are staged, nothing to send to the AI"**

Everything staged matched an ignore rule. Check your `.xommitignore`.

**"Could not reach AI models"**

The request never reached the provider. Check your connection, proxy settings, and firewall.

**"rate limit or quota exceeded (429)"**

You hit the provider limit. Wait, use a different model, or use `--model` to switch to a cheaper one.

Next: [commands.md](commands.md)
